mkdir ..\res\shader\Compiled

call .\dxc.exe -Fh "..\res\shader\Compiled\SimpleVS.inc" -T vs_6_6 -E VSFunc "..\res\SimpleVS.hlsl"
call .\dxc.exe -Fh "..\res\shader\Compiled\SimplePS.inc" -T ps_6_6 -E PSFunc "..\res\SimplePS.hlsl"

