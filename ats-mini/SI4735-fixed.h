#include <SI4735.h>

class SI4735_fixed: public SI4735
{
  public:
    // Clear stale SSB sideband bits before the first FM tune
    void setFM(uint16_t fromFreq, uint16_t toFreq, uint16_t initialFreq, uint16_t step)
    {
      currentFrequencyParams.arg.USBLSB = 0;
      SI4735::setFM(fromFreq, toFreq, initialFreq, step);
    }

    // AM komplett ohne die 150-kHz-Sperre der Basisbibliothek initialisieren
    void setAM(uint16_t fromFreq, uint16_t toFreq, uint16_t initialFreq, uint16_t step)
    {
      currentFrequencyParams.arg.USBLSB = 0;
      
      // Untere Grenze auf 100 kHz öffnen
      if (fromFreq < 100) fromFreq = 100;

      // Basisaufruf mit dem korrigierten Startwert
      SI4735::setAM(fromFreq, toFreq, initialFreq, step);
      
      // Erzwinge die Frequenz direkt, falls sie unter dem harten Bibliothekslimit liegt
      if (initialFreq < 150) {
        setFrequency(initialFreq);
      }
    }

    // AM patch loading without SSB-specific properties. A null content
    // pointer restores stock AM when setAM() would otherwise skip power-up.
    void loadAMPatch(const uint8_t *content, uint16_t size)
    {
      if(content)
      {
        queryLibraryId();
        setPowerUp(ctsIntEnable, 0, 1, currentClockType, AM_CURRENT_MODE, currentAudioMode);
        radioPowerUp();
        delay(50);
        downloadPatch(content, size);
        delay(25);
      }
      else
      {
        powerDown();
        setPowerUp(ctsIntEnable, 0, 0, currentClockType, AM_CURRENT_MODE, currentAudioMode);
        radioPowerUp();
      }
      setAvcAmMaxGain(currentAvcAmMaxGain);
      setVolume(volume);
      currentSsbStatus = 0;
      currentFrequencyParams.arg.USBLSB = 0;
      lastMode = AM_CURRENT_MODE;
    }

    // Fixing SI4735::getRdsPI() bug where it only returns BLOCKAL
    uint16_t getRdsPI(void)
    {
      if(getRdsReceived() && getRdsNewBlockA())
        return (currentRdsStatus.resp.BLOCKAH << 8) + currentRdsStatus.resp.BLOCKAL;
      else
        return 0x0000;
    }

    // Fixing SI4735::getRdsProgramType() bug where it only returns three lower bits
    uint8_t getRdsProgramTypeX(void)
    {
      uint16_t blockB = (currentRdsStatus.resp.BLOCKBH << 8) + currentRdsStatus.resp.BLOCKBL;
      return (blockB >> 5) & 0x1F;
    }

    // Fixing SI4735::getRdsText2A() which does not follow version bit
    char *getRdsText2A(void)
    {
      return getRdsVersionCode()? NULL : SI4735::getRdsText2A();
    }

    // Fixing SI4735::getRdsText2B() which does not follow version bit
    char *getRdsText2B(void)
    {
      return getRdsVersionCode()? SI4735::getRdsText2B() : NULL;
    }

    // Only one kind of text is available
    inline char *getRdsProgramInformation(void)
    {
      return getRdsVersionCode()? SI4735::getRdsText2B() : SI4735::getRdsText2A();
    }

    // Only one kind of text is available
    inline char *getRdsStationInformation(void)
    {
      return getRdsVersionCode()? SI4735::getRdsText2B() : SI4735::getRdsText2A();
    }

    // Implementing the empty SI4735::setFmStereoOff() placeholder by
    // moving every blend threshold beyond reach, which pins the audio to mono
    void setFmStereoOff()
    {
      setFmBlendRssiStereoThreshold(127);
      setFmBLendRssiMonoThreshold(127);
      setFmBlendSnrStereoThreshold(127);
      setFmBLendSnrMonoThreshold(127);
      setFmBlendMultiPathStereoThreshold(0);
      setFmBlendMultiPathMonoThreshold(0);
    }

    // Implementing the empty SI4735::setFmStereoOn() placeholder by restoring
    // the blend thresholds the chip starts up with, see AN332
    void setFmStereoOn()
    {
      setFmBlendRssiStereoThreshold(49);
      setFmBLendRssiMonoThreshold(30);
      setFmBlendSnrStereoThreshold(27);
      setFmBLendSnrMonoThreshold(14);
      setFmBlendMultiPathStereoThreshold(20);
      setFmBlendMultiPathMonoThreshold(60);
    }

    // Decode UTC time directly from the RDS data blocks.
    bool getRdsUTCEpoch(uint32_t *epoch)
    {
      if(!epoch || getRdsGroupType() != 4 || getRdsVersionCode()) return(false);

      uint16_t blockB = (currentRdsStatus.resp.BLOCKBH << 8) | currentRdsStatus.resp.BLOCKBL;
      uint16_t blockC = (currentRdsStatus.resp.BLOCKCH << 8) | currentRdsStatus.resp.BLOCKCL;
      uint16_t blockD = (currentRdsStatus.resp.BLOCKDH << 8) | currentRdsStatus.resp.BLOCKDL;
      uint32_t mjd = ((uint32_t)(blockB & 0x0003) << 15) | (blockC >> 1);
      uint8_t hour = ((blockC & 0x0001) << 4) | (blockD >> 12);
      uint8_t minute = (blockD >> 6) & 0x003F;

      if(hour > 23 || minute > 59)
        return(false);

      const uint32_t unixEpochMJD = 40587;
      uint32_t seconds = (hour * 60 + minute) * 60;

      if(mjd < unixEpochMJD || mjd - unixEpochMJD > (UINT32_MAX - seconds) / 86400)
        *epoch = seconds;
      else
        *epoch = (mjd - unixEpochMJD) * 86400 + seconds;

      return(true);
    }

  void seekStationProgress(void (*showFunc)(uint16_t f), bool (*stopSeeking)(), uint8_t up_down)
  {
    si47x_frequency freq;
    long elapsed_seek = millis();

    // seek command does not work for SSB
    if (lastMode == SSB_CURRENT_MODE)
      return;

    seekStation(up_down, 0);
    do
    {
      delay(maxDelaySetFrequency);
      getStatus(0, 0);
      delay(maxDelaySetFrequency);
      freq.raw.FREQH = currentStatus.resp.READFREQH;
      freq.raw.FREQL = currentStatus.resp.READFREQL;
      currentWorkFrequency = freq.value;
      if (showFunc != NULL)
        showFunc(freq.value);
      if (stopSeeking != NULL)
        if (stopSeeking())
          return;

    } while (!currentStatus.resp.VALID && !currentStatus.resp.BLTF && (millis() - elapsed_seek) < maxSeekTime);
  }
};
