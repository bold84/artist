
When building on architectures other than x86/x86_64, vcpkg requires the environment variable VCPKG_FORCE_SYSTEM_BINARIES to be set. Certain tools will then not be downloaded but must be available on the host.

For Ubuntu aarch64:

sudo apt install autoconf python2 python3 pip 
sudo pip3 install ninja meson

Installing via pip is necessary because the versions that are available via apt in Ubuntu 20.04 did not meet vcpkg's requirements.
