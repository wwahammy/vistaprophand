This project is abandoned has not been used or supported for quite some time. Enjoy if it helps you though!

------------------------------------------------------------------------

		Vista Metadata Project Install

NOTE: Use this code at your own risk! It's worked fine in my limited testing but you never know. I'm not responsible if it wrecks your files, metadata or marriage.


Requirements:
- Windows Vista
- Visual Studio 2008 (It MIGHT work on Express but no promises)
- Windows Installer XML v3

This code is based on the XMP Library from Adobe so the install is very similar.


1. Download the Windows SDK. This HUGE download (1.3GB.. ugh) contains all the headers and libraries needed to compile this. To build you only need the headers and libraries, so you can ignore all the samples and documentation if you'd like. Add the lib and include folders from the Windows SDK to the VC++ Directories in the Visual Studio Options.

It can be found at http://www.microsoft.com/downloads/details.aspx?familyid=C2B1E300-F358-4523-B479-F53D234CDCCF&displaylang=en

2. Download the Apple Quicktime SDK for Windows (http://developer.apple.com/sdk/). Install it and copy the CIncludes and Libraries folders to the src/xmp/third-party/QTDevWin folder.
 
There is no Quicktime support yet so in reality there's no real reason to need the SDK BUT I don't see the point of taking something out that I'll need to add back in later. Therefore, you need the Quicktime SDK, otherwise this won't compile.

3. Download the DirectX 9 SDK (not the general for some reason that SDK is missing a needed header file). Without it, the thumbnail provider won't compile and babies might cry or something. Add the lib and include folders from the DX SDK to the VC++ Directories in the Visual Studio Options.

It can be found at http://www.microsoft.com/downloads/details.aspx?FamilyID=77960733-06e9-47ba-914a-844575031b81&DisplayLang=en

4. Open Visual Studio and build the solution in debug mode. (Release doesn't work/haven't really tried but if someone wants to and write back to me please do so!)

5. Build the WiX project.

6. Go to build\WiXInstaller\Debug directory and run the msi.

7. If you already have AVI files with metadata that you'd like indexed, you need to rebuild your Windows Search index to have them indexed. All changes from now on will be recorded in the index like they would for any file.

-------------------------

				UNINSTALL

1. Uninstall from the control panel like you would any program.

IMPORTANT NOTE: This should reinstall the Windows built-in AVI property handler and thumbnail handlers and their registry settings but sometimes it doesn't. I don't know why.


