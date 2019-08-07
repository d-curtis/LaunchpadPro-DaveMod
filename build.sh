echo "-    -    -    -"
echo "Running make"
make
echo "Flashing firmware"
sendmidi dev launchpad syf build/launchpad_pro.syx
echo "Done!"
echo "-    -    -    -"