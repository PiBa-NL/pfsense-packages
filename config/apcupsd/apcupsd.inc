<?php
/* $Id$ */
/* ========================================================================== */
/*
    apcupsd.inc
    part of the apcupsd package for pfSense
    Copyright (C) 2013 Danilo G. Baio <dbaio@bsd.com.br>

    All rights reserved.            
			                                                                  */
/* ========================================================================== */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
                                                                              */
/* ========================================================================== */
require_once("util.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("globals.inc");

function php_install_apcupsd(){
	sync_package_apcupsd();	
}

function php_deinstall_apcupsd(){
	global $config, $g;

	conf_mount_rw();
	$pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
	if ($pfs_version > 2.0){
		define('APCUPSD_BASE', '/usr/pbi/apcupsd-' . php_uname("m"));
	} else {
		define('APCUPSD_BASE', '/usr/local');
	}

	exec("/usr/bin/killall apcupsd");
	unlink_if_exists(APCUPSD_BASE . "/etc/rc.d/apcupsd.sh");
	unlink_if_exists(APCUPSD_BASE . "/etc/apcupsd/apcupsd.conf");
	unlink_if_exists("/var/log/apcupsd/apcupsd.log");
	unlink_if_exists("/var/run/apcupsd/apcupsd.pid");

	if (is_dir("/var/log/apcupsd"))
		exec("/bin/rm -r /var/log/apcupsd/");
	if (is_dir("/var/run/apcupsd"))
		exec("/bin/rm -r /var/run/apcupsd/");

   conf_mount_ro();
}

function validate_input_apcupsd($post,&$input_errors){

	if  (isset($post['apcupsdenabled'])){
	
		if ($post['polltime'] != '' && !is_numericint($post['polltime'])) {
			$input_errors[]='Poll Time is not numeric.';
		}

		if ($post['onbatterydelay'] != '' && !is_numericint($post['onbatterydelay'])) {
			$input_errors[]='OnBattery Delay is not numeric.';
		}

		if ($post['batterylevel'] != '' && !is_numericint($post['batterylevel'])) {
			$input_errors[]='Battery Level is not numeric.';
		}

		if ($post['minutes'] != '' && !is_numericint($post['minutes'])) {
			$input_errors[]='Minutes is not numeric.';
		}

		if ($post['timeout'] != '' && !is_numericint($post['timeout'])) {
			$input_errors[]='Timeout is not numeric.';
		}

		if ($post['annoy'] != '' && !is_numericint($post['annoy'])) {
			$input_errors[]='Annoy is not numeric.';
		}

		if ($post['annoydelay'] != '' && !is_numericint($post['annoydelay'])) {
			$input_errors[]='Annoy Delay is not numeric.';
		}

		if ($post['killdelay'] != '' && !is_numericint($post['killdelay'])) {
			$input_errors[]='Kill Delay is not numeric.';
		}

		if ($post['nisip'] != '') {
			if (!is_ipaddr_configured($post['nisip']) && !preg_match("/(127.0.0.1|0.0.0.0)/",$post['nisip'])) {
				$input_errors[]='NIS Ip is not a configured IP address.';
			}
		}

		if ($post['nisport'] != '') {
			if (!preg_match("/^\d+$/", $post['nisport'])) {
				$input_errors[]='NIS Port is not numeric.';
			}	
		}

	} // apcupsdenabled
}

function sync_package_apcupsd(){
	global $config, $g;

	conf_mount_rw();

	// check pfsense version
	$pfs_version = substr(trim(file_get_contents("/etc/version")),0,3);
	if ($pfs_version > 2.0){
		define('APCUPSD_BASE', '/usr/pbi/apcupsd-' . php_uname("m"));
		}
	else {
		define('APCUPSD_BASE', '/usr/local');
	}

	// check apcupsd settings
	if (is_array($config['installedpackages']['apcupsd'])){
		$apcupsd_config = $config['installedpackages']['apcupsd']['config'][0];
		if ($apcupsd_config['apcupsdenabled']=="on"){
			$upsname=$apcupsd_config['upsname'];
			$upscable=$apcupsd_config['upscable'];
			$upstype=$apcupsd_config['upstype'];
			$polltime=($apcupsd_config['polltime'] != ''? $apcupsd_config['polltime'] : "60");
			$onbatterydelay=($apcupsd_config['onbatterydelay'] != ''? $apcupsd_config['onbatterydelay'] : "6");
			$batterylevel=($apcupsd_config['batterylevel'] != ''? $apcupsd_config['batterylevel'] : "5");
			$minutes=($apcupsd_config['minutes'] != ''? $apcupsd_config['minutes'] : "3");
			$timeout=($apcupsd_config['timeout'] != ''? $apcupsd_config['timeout'] : "0");
			$annoy=($apcupsd_config['annoy'] != ''? $apcupsd_config['annoy'] : "300");
			$annoydelay=($apcupsd_config['annoydelay'] != ''? $apcupsd_config['annoydelay'] : "60"); 
			$killdelay=($apcupsd_config['killdelay'] != ''? $apcupsd_config['killdelay'] : "0");
			$netserver=$apcupsd_config['netserver'];
			$nisip=($apcupsd_config['nisip'] != ''? $apcupsd_config['nisip'] : "0.0.0.0");
			$nisport=($apcupsd_config['nisport'] != ''? $apcupsd_config['nisport'] : "3551");
			$upsclass=$apcupsd_config['upsclass'];
			$upsmode=$apcupsd_config['upsmode'];
	
			include("/usr/local/pkg/apcupsd.conf.php");
			file_put_contents(APCUPSD_BASE . "/etc/apcupsd/apcupsd.conf", $apcupsdconf, LOCK_EX);
		}
	}

	// RC FILE	
	$apcupsd_rcfile="/usr/local/etc/rc.d/apcupsd.sh";
	if (is_array($apcupsd_config) && $apcupsd_config['apcupsdenabled']=="on"){
		$apcupsd_start = "echo \"Starting APC UPS Daemon...\"\n";
		if ($apcupsd_config['killonpowerfail']=="on"){
			$apcupsd_start .= "	" . APCUPSD_BASE . "/sbin/apcupsd --kill-on-powerfail";
		}else{
			$apcupsd_start .= "	" . APCUPSD_BASE . "/sbin/apcupsd";
		}
	
		$apcupsd_stop  = "echo \"Stopping APC UPS Daemon...\"\n";
		$apcupsd_stop .= "	/usr/bin/killall apcupsd\n";
		$apcupsd_stop .= "	/bin/sleep 5";
	
		/* write out rc.d start/stop file */
		write_rcfile(array(
						"file" => "apcupsd.sh",
						"start" => "$apcupsd_start",
						"stop" => "$apcupsd_stop"
				)
		);
		mwexec("{$apcupsd_rcfile} restart");	
	}else{
		if (file_exists($apcupsd_rcfile)){
			mwexec("{$apcupsd_rcfile} stop");
			unlink($apcupsd_rcfile);
		}
	}
		
	conf_mount_ro();
}
?>
