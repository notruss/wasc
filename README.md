# WASC
WASC is Windows Audio Services (command line interface) Client.
Simple command line interface utility for MS Windows to get raw audio data sent to playback audio endpoint device using WASAPI loopback feature.
It is built using MinGW64 now, but code is trivial so any C compiler will do the job.
Wasc.exe without parameters prints a list of endpoint audio devices with ID strings of kind {...} which are used to identify devices inside windows audio infrastructure following by stream format.
Next just run wasc.exe {id-of-device} to get raw audio on it's stdout.
For example to get audio playing on your super soundcard saved to record-output.mp3 do...

## Usage/Examples

```javascript
>wasc.exe
{0.0.0.00000000}.{12345678-1234-1234-1234-123456789012} eRender "Super Wave Out Device"
\-EXT(0xfffe), 32 b/sa, 2 ch, 8 B/frame, 48000 sa/s, float, 32 valid b/sa, ch mask=0x3, 3000 Kib/s
{0.0.1.00000000}.{12345678-1234-1234-1234-123456789456} eCaptur "Recording Mixer"
\-EXT(0xfffe), 32 b/sa, 2 ch, 8 B/frame, 44100 sa/s, float, 32 valid b/sa, ch mask=0x3, 2756 Kib/s

>wasc.exe {0.0.0.00000000}.{12345678-1234-1234-1234-123456789012} | ffmpeg -f f32le -ac 2 -ar 48000 -i pipe:0 -c:a libmp3lame -b:a 256k record-output.mp3
```
## Poll driven transfer
Since event driven transfer doesn't work on Windows 7(and others I guess) wasc has --poll option followed by number of milliseconds between buffer polls for data. This is not very reliable in practice since Win7 thread scheduler is not much aware about real time streams processing. So even having poll interval set like 1 ms one can have 20 or even 100 ms timeouts eventually loosing audio data. Use Win10+ and event driven transfer.
