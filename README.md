# NFC as a Gameplay Controller - Research Project

## Introduction

We've all had that experience as a kid. You buy a toy, you bring it home, and somehow it magically connects to a video game. No cables, no setup, just place it on a platform and your character appears on screen. That's the promise of **toys to life** games, and it felt like actual magic when Skylanders introduced it in 2011.

But what if that magic didn't have to stay locked inside a proprietary product?

The hardware is still out there. You can find Skylanders portals for a couple of euros at flea markets. NFC chips cost less than a coffee. And yet, no open tool exists that lets an independent developer recreate that experience in their own game.

**This project explores whether it's possible to build an open-source Unreal Engine 5 plugin that turns a standard NFC reader into a gameplay controller usable by any developer, without any proprietary hardware.**

---

## State of the Art

### <ins>A brief history of toys-to-life :</ins>

<img width="864" height="768" alt="skylanders-spyros-adventure-promo-01" src="https://github.com/user-attachments/assets/4b1d2c65-24ab-4385-8c01-24254d654ed9" />

The story starts in 2011 with **Skylanders: Spyro's Adventure** by Activision. The concept was deceptively simple: buy a figurine, place it on a glowing plastic portal, and your character appears in the game. The figurine itself stored your character's progression level, experience, gear inside a tiny chip embedded in the base. Take it to a friend's house, place it on their portal, and your character was there too with all your progress intact.

It was a hit. Activision sold over $500 million worth of figurines in the first year alone. Naturally, everyone wanted a piece of it.

| Game | Years | What made it unique |
|---|---|---|
| **Skylanders** | 2011–2016 | First toys-to-life game, Mifare Classic chip stores progression inside the figurine |
| **Disney Infinity** | 2013–2016 | Disney and Pixar characters, sandbox gameplay with creative tools |
| **LEGO Dimensions** | 2015–2017 | You physically built the vehicle or gadget with LEGO bricks before using it in-game |
| **Nintendo Amiibo** | 2014–present | Simpler NFC tag, read by the Switch or 3DS, still actively supported today |

Every single one of these is **completely proprietary**. No public API, no documentation, no open standard. The magic is real, but it belongs entirely to the companies that built it.

### <ins>So, how does NFC actually work?</ins>

Before going further, it's worth understanding the technology behind all of this because it's genuinely fascinating.

**NFC** stands for Near Field Communication. It's a wireless communication technology that works at a distance of a few centimetres, using radio waves at a frequency of **13.56 MHz**. You've used it without thinking about it: every time you tap your bank card on a payment terminal, that's NFC.

<img width="700" height="189" alt="ST-Introduction-NFC-1" src="https://github.com/user-attachments/assets/7b3e8185-4a7f-40bb-8554-e5f42a6792b4" />

Here's how it works in practice:

The **reader** (the portal, or in our case a USB device) generates a continuous electromagnetic field. When an NFC **tag** (the chip inside the figurine or sticker) enters that field, something clever happens: the tag harvests energy directly from the electromagnetic field itself to power up its tiny microchip. No battery needed. Once powered, the tag and reader communicate by modulating that same field back and forth.

This is why NFC tags are so cheap they're completely passive. No battery, no active electronics, just a copper coil antenna wrapped around a microscopic chip.

<img width="800" height="450" alt="sb-main800-45" src="https://github.com/user-attachments/assets/2f36ec75-3bde-49ab-b9e1-62dcc2ceb714" />

Every NFC tag has a **UID** a Unique IDentifier. It's a sequence of 7 bytes (56 bits) that is permanently burned into the chip during manufacturing and can never be changed. Think of it as the tag's fingerprint. This UID is what our plugin reads to identify which figurine was placed on the reader.

### <ins>The Skylanders portal specifically :</ins>

The portal Skylanders used is a custom piece of hardware that connects via USB. Inside, it has **three independent antennas** one per slot on the platform surface. Each antenna generates its own RF field, which is why you can place three figurines simultaneously without them interfering with each other.

<img width="1000" height="750" alt="k3kh1taxy3191" src="https://github.com/user-attachments/assets/2c782c36-9b04-4bfc-b68b-fcf342c92bcf" />

The figurines themselves contain a **Mifare Classic** chip a slightly more advanced NFC tag that not only has a UID, but also has writable memory. This is where Skylanders stored your character's experience points, level, and equipment directly on the physical figurine. The game didn't save to the console it saved to the toy.

From the computer's perspective, the portal appears as a **HID device** the same category as keyboards and mice. It uses a completely custom, undocumented communication protocol that Activision never published. Everything we know about it comes from community reverse-engineering, primarily from the **Dolphin Emulator** team who needed to support it for Wii emulation.

### <ins>What already existed before this project :</ins>

The Skylanders portal wasn't completely ignored by the open-source world. A few community projects documented its behavior:

- **Dolphin Emulator** reverse-engineered the full portal protocol to allow playing Skylanders on PC via Wii emulation. Their C++ source code is the best existing reference, available on GitHub under GPLv2.
- **SkyReader** — a community tool that lets you read and write the RFID chips inside Skylanders figurines.
- **Skynfc** — a small Python script showing basic portal communication from a PC.

None of these were designed to be integrated into a game engine. They're research tools, not developer tools.

On the game engine side, NFC support for desktop/PC is essentially nonexistent. Unity has no NFC module for PC. Unreal Engine has no NFC module at all. The documentation for both engines doesn't even mention the use case.

---

## Approach

### <ins>The original plan :</ins>

The initial idea was to use the Skylanders portal directly, relying on the HID protocol that the Dolphin community had reverse engineered. It felt poetic use the original hardware, stay close to the source material.

After spending a few weeks studying the protocol and attempting an implementation, I ran into three walls that were hard to ignore:

First, the protocol is completely undocumented by Activision. Everything relies on community reverse engineering that may not be complete or fully accurate. Building on that foundation means building on sand.

Second, on modern Windows, accessing a HID device from software requires either Windows letting you through (it doesn't by default for vendor-specific devices) or installing a third-party driver called WinUSB via a tool called Zadig. That immediately makes the experience worse for anyone who wants to use the plugin they'd have to manually install a driver before anything works.

Third, and most honestly: debugging a custom binary HID protocol inside Unreal Engine's build system, with limited documentation, looked like it would take most of the project's remaining time. The research would become about the protocol, not about the gameplay.

### <ins>The pivot :</ins>

<img width="1500" height="1500" alt="20121229145412acr122u_front_2048x2048" src="https://github.com/user-attachments/assets/0554571c-da06-431d-9caa-db908f4056d0" />

The solution was a standard NFC reader the **ACR122U** by ACS combined with plain **NTAG213 stickers**. This combination costs about 25€ for the reader and less than 0.20€ per sticker. More importantly, it uses a standard protocol called **PC/SC** (Personal Computer/Smart Card) that Windows supports natively through a system DLL called `winscard.dll`.

No driver install. No reverse-engineering. Plug it in, and Windows already knows how to talk to it.

The gameplay logic doesn't change at all. Whether the physical object is a Skylanders figurine or a plain NFC sticker, what the plugin actually reads is the UID that 7-byte fingerprint we talked about earlier. The plugin looks up that UID in a table, finds the associated character or level, and triggers the right gameplay response. The figurine's design is irrelevant to the code.

| | Skylanders Portal (HID) | Standard Reader (PC/SC) |
|---|---|---|
| Protocol | Proprietary, undocumented | Standard, Microsoft-documented |
| Driver install | Required (WinUSB via Zadig) | None, native Windows |
| Hardware cost | 5-15€ secondhand | ~25€ new |
| Tag cost | Skylanders figurines only | Any NFC sticker, ~0.20€ |
| Multi-tag | 3 simultaneous (3 antennas) | 1 at a time (1 antenna) |
| Open-source reference | Dolphin Emulator | Full Microsoft documentation |

The one real trade off is multi-tag support. The ACR122U has a single antenna. Two tags in its field simultaneously collide on the same radio frequency and neither gets decoded. This is a physical constraint no software can fix it. The Skylanders portal's ability to read three figurines at once comes from its three independent antennas, which is why it was custom hardware in the first place.

For this project, one active tag at a time is a perfectly reasonable constraint. One character, one world, one action.

**The door to Skylanders remains open.** The plugin is built around an abstract interface called `INFCReaderBase`. Today, the PC/SC reader implements that interface. In the future, a Skylanders HID reader could implement the exact same interface and slot in without touching any gameplay code or Blueprint.
