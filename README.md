# EC_PRO-LAN



Anti-Cheats: (FACEIT, (ESEA w/ custom version)  
Early 2019 - late 2020. R.I.P. https://h0mbre.github.io/RyzenMaster_CVE/#  

# Requirements:
Windows 10 Enterprise 1607 LTSB with all updates  
AMD Ryzen CPU  
Motherboard B350-B450 (B550 client is bugged)  
Logitech GHUB installed ( for mouse input )


# Older gen ryzen processors has maybe different Version of AmdRyzenMaster driver (1.3.0.0)
you have to then replace https://github.com/ekknod/EC_PRO-LAN/blob/main/client_windows/server.cpp#L65  
with this:  
unsigned char b_amd[] = {
        0x5C, 0x00, 0x44, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00, 0x5C, 0x00, 0x41, 0x00, 0x4D, 0x00,
        0x44, 0x00, 0x52, 0x00, 0x79, 0x00, 0x7A, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x4D, 0x00, 0x61, 0x00, 0x73, 0x00, 0x74, 0x00,
        0x65, 0x00, 0x72, 0x00, 0x44, 0x00, 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x56, 0x00, 0x31, 0x00,
        0x33, 0x00, 0x00, 0x00
    } ;
 


# Installation


precompiled client: https://www.unknowncheats.me/forum/downloads.php?do=file&id=31551  
copy opengl32.dll to C:\Program Files\AMD\RyzenMaster\bin  
open amdryzenmaster utility, allow firewall  
open csgo.exe  
open client_private.exe in separate PC in your LAN network. 



# Vulnerabilities
Logitech input manipulation with their macro driver (works still ESEA/FACEIT, will work as long as logitech macros)
Copying game memory with AmdRyzenMaster vulnerability  

# Why releasing?
Because someone else found same vulnerability and made it public: https://h0mbre.github.io/RyzenMaster_CVE/#  

# Youtube Video (EC_PRO LAN android client (wifi))
[![IMAGE ALT TEXT](http://i3.ytimg.com/vi/l91pJW86KEQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=l91pJW86KEQ "EC_PRO lan (android client)")


# Youtube Video (EC_PRO lan raspberry pi client)
[![IMAGE ALT TEXT](http://i3.ytimg.com/vi/qrUvuK8Hxq8/maxresdefault.jpg)](https://www.youtube.com/watch?v=qrUvuK8Hxq8&feature=youtu.be "EC_PRO lan (rasberry client)")
