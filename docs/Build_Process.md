#Instructions for building firmware from source:

Prerequisites:

KinectIP has latest version of Debian Stretch installed (available here: https://github.com/ayufan-rock64/linux-build/releases/download/0.8.3/stretch-minimal-rock64-0.8.3-1141-arm64.img.xz)

- Remote Hostname is set to 'KinectIP' (modify /etc/hostname & /etc/hosts)
- Remote Password for user Rock64 is changed from default
- Root SSH is enabled (preferably by key authentification only) and your local computer can ssh into the Remote computer without a password prompt.

INSTRUCTIONS: (run on local computer terminal)
- ssh rock64@rock64
- sudo nano /etc/hostname
- # change line 1 to KinectIP, save and exit
- sudo nano /etc/hosts
- # change line 2 to 127.0.1.1 KinectIP
- passwd # change rock64 user password for increased security
- sudo passwd # this enables the root account (this password can be as complex as you want, because we will be setting up ssh key authentification instead.)
- sudo reboot
-----------------
- ssh rock64@KinectIP
- sudo nano /etc/ssh/sshd_config
- # change line #permit root login to 'yes' and uncomment it (temporarily)
- sudo service ssh restart
- exit
- ssh-keygen # if not done already on local computer
- ssh-copy-id -i /home/"$USERNAME"/.ssh/id_rsa root@KinectIP
- ssh root@KinectIP
- nano /etc/ssh/sshd_config
- # change line #permit root login to 'prohibit-password'
- service ssh restart
- exit

You can now execute build scripts on your local computer which cross-compile and transfer files to the remote computer!

run the following build scripts from the 'KinectIP' directory of the git repository:

  ./deploy_scripts/install_full.sh

Note: remote filesystem must be mounted in WSL before the script can be run. If on native Linux, SSHFS can be used.
For WSL users, mount the root directory using SFTP Net Drive (https://www.nsoftware.com/sftp/drive/download.aspx)
in Windows, then run [sudo mount -t drvfs Z: /mnt/rock64] in WSL

In future revisions the mounting process will be automatic



