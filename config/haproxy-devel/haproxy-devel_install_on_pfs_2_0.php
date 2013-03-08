<?php
/*
	Workaround to install haproxy 1.5dev7 on pfSense 2.0.

	-Install the package "System Patches" and add the a new 'patch'
		Desciption:			"haproxy-devel binary installer"
		URL/Commit ID:		* leave this field empty *
		Patch contents:		copy the whole content of this file into a new "patch contents"
		Path strip count:	3
		Base Directory:		/usr/local/www
		Ignore whitespace:	* leave this field empty *
	
	-Apply the patch.
	-Open the url (with the IP of your pfSense box): https://192.168.1.1/haproxy-devel_install_on_pfs_2_0.php
		! This will IMMEDIATELY start installing the package !
		
	And will tell you when its "Done"
	Leave the page open while its bussy.
		
	You now have installed the haproxy 1.5dev17 haproxy-devel package.
	
	Configure haproxy as required for your deplyment using the new "Services\HAProxy" menu in pfSense.
*/

require_once("guiconfig.inc");
include_once('pfsense-utils.inc');
include_once("pkg-utils.inc");

function write_bpi2bz2_script($scriptfilename) {
	$pbi2bz2 = <<<EOD
#!/bin/sh
# PC-BSD PBI to tar.bz2 convertor
if [ $# -ne 1 ] ; then
echo "Usage: $(basename $0) file"; exit 1
fi
if [ ! -r $1 ] ; then
echo "Error: $1: File doesn't exist or is not readable." ; exit 1
fi
PKG="$(basename $1 .pbi).tar.bz2"
printf "Converting PBI -> tar.bz2... "
tail +$(awk '/__PBI_ARCHIVE__/ {print NR+1}' $1) $1 >\${PKG} 2>&1
if [ $? -ne 0 ] ; then
printf "\nError: Couldn't convert PBI file. PBI is probably corrupt.\n"
rm \${PKG}
exit 1
fi
echo "Done."

EOD;
	$fd = fopen($scriptfilename, "w");
	fwrite($fd, $pbi2bz2);
	fclose($fd);
}

function pfSense2_0_installPBI() {
	write_bpi2bz2_script("/tmp/pbi2bz2.sh");

	$arch = php_uname("m");
	$urlarch = ($arch == "i386") ? "" : $arch . '/';
	$rel = get_freebsd_version();
	$priv_url = "http://files.pfsense.org/packages/{$urlarch}{$rel}/All/";

	download_file_with_progress_bar("$priv_url/haproxy-devel-1.5-dev17-$arch.pbi","/tmp/haproxy-devel-1.5-dev17.pbi");

	echo "Extracting files from pbi.";
	$commands = <<<EOD
cd /tmp
chmod +x pbi2bz2.sh
./pbi2bz2.sh haproxy-devel-1.5-dev17.pbi
mkdir /tmp/OUT
echo ##CD
cd /tmp/OUT/
echo ##TAR
tar jfx ../haproxy-devel-1.5-dev17.tar.bz2 sbin
cp ./sbin/haproxy /usr/local/sbin/haproxy

EOD;
	`$commands`;
}

?>
<html>
<body>
<strong>Installing HAProxy-devel package.</strong><br/>
<table id="progholder" name="progholder" height='15' width='630' border='0' colspacing='0' cellpadding='0' cellspacing='0'>
	<td background="/themes/the_wall/images/misc/bar_gray.gif" valign="top" align="left">
		<img src='/themes/the_wall/images/misc/bar_blue.gif' width='0' height='15' name='progressbar' id='progressbar'>
	</td>
</table>
<form>
	<textarea name='output' id='output' rows="10" cols="90">
	</textarea>
</form>
<br/>
<?
install_package('haproxy-devel');
?>
<strong>Downloading and installing HAProxy 1.5dev17 binaries.</strong><br/>
This can take another minute..<br/>
<?
pfSense2_0_installPBI();
?>
<br/>
Installation done.<br/>
haproxy should show its current installed version below:<br/>
<br/>
<strong>
<?=`haproxy`?>
</strong>
<br/>
<br/>
The installation is done.<br/>
You can now return to pfSense <a href="/">pfSense home page</a>
</body>
</html>
