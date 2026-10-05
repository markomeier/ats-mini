Neuestes Release: v1.01 mit folgenden Änderungen:

🛠️ Changelog / Versionshinweise
📻 Skala & Tuning-Optik (Klassischer Röhrenradio-Stil)
Design-Anpassungen:
Roter Indikator-Kreis (x=160, y=140) und prägnanter roter Zeigerstrich (2\text{ px} breit) im klassischen Röhrenradio-Look.

Horizontale Führungslinien an der Skalen-Ober- und Unterkante.
Hervorgehobene, doppelte 10\text{er}-Hauptstriche sowie abgestufte 5\text{er}- und 1\text{er}-Markierungen für bessere Lesbarkeit.
Dynamisches Frequenz-Feedback:
Die Frequenzeinheit (MHz / kHz) wechselt beim Drehen des Abstimmknopfs (Tuning) automatisch für 800\text{ ms} auf Gelb und schaltet danach wieder auf Neongrün um.

📊 S-Meter & Signal-Anzeige
Peak-Hold Funktion:
Integrierter Peak-Hold-Effekt im S-Meter: Der höchste gemessene Signalbalken wird stets in Rot gehalten und fällt nach 1{,}2\text{ s} schrittweise ab.
Signalwerte als RDS-Fallback:
Wenn kein RDS-Text vorhanden ist, werden in der einzeiligen Textansicht automatisch die aktuellen Empfangswerte in Neongrün dargestellt (S: XX dBuV | SNR: XX dB).

🚀 Code-Optimierungen & Refactoring
Entfernung veralteter Deklarationen:

Direkter Empfänger-Zugriff:
Abfrage der Signalwerte direkt über das globale Receiver-Objekt

_________________

V1.00 Mod

Dies ist die uebersetzte Originalversion ins Deutsche mit einigen 
Grafischen Änderungen für bessere Lesbarkeit (Da ich selbst schlecht sehen kann)

Vielen herzlichen Dank dann die originalen Entwickler.

Infos zu den Entwicklern und Anleitungen siehe unten.


# ATS Mini

![](docs/source/_static/esp32-si4732-ui-theme.jpg)

This firmware is for use on the SI4732 (ESP32-S3) Mini/Pocket Receiver

Based on the following sources:

* Volos Projects:    https://github.com/VolosR/TEmbedFMRadio
* PU2CLR, Ricardo:   https://github.com/pu2clr/SI4735
* Ralph Xavier:      https://github.com/ralphxavier/SI4735
* Goshante:          https://github.com/goshante/ats20_ats_ex
* G8PTN, Dave:       https://github.com/G8PTN/ATS_MINI

## Releases

Check out the [Releases](https://github.com/esp32-si4732/ats-mini/releases) page.

## Documentation

The hardware, software and flashing documentation is available at <https://esp32-si4732.github.io/ats-mini/>

## Discuss

* [GitHub Discussions](https://github.com/esp32-si4732/ats-mini/discussions) - the best place for feature requests, observations, sharing, etc.
* [TalkRadio Telegram Chat](https://t.me/talkradio/174172) - informal space to chat in Russian and English.
