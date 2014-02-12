#pragma once

#include <set>
#include <string>
#include <iwlib.h>

#include "exec.hpp"

std::set<std::string> iwlist(void) 
{
	std::set<std::string> res;

	wireless_scan_head head;
	wireless_scan *result;
	iwrange range;
	int sock;


	/* Open socket to kernel */
	sock = iw_sockets_open();

	/* Get some metadata to use for scanning */
	if (iw_get_range_info(sock, "wlan0", &range) < 0) {
		printf("Error during iw_get_range_info. Aborting.\n");
		return res;
	}

	/* Perform the scan */
	if (iw_scan(sock, "wlan0", range.we_version_compiled, &head) < 0) {
		printf("Error during iw_scan. Aborting.\n");
		return res;
	}

	/* Traverse the results */
	result = head.result;
	while (NULL != result) {
		std::stringstream ss;
		ss << result->b.essid;
		//if ((result->b.key_flags & IW_ENCODE_DISABLED) == IW_ENCODE_DISABLED)
        if (result->b.has_key)
            ss << " enc";
		res.insert(ss.str());
		
        result = result->next;
	}

	return res;
}

enum ENC
{
	NONE,
	WEP,
	WPA
};

ENC encType(const std::string& enctype)
{
	ENC res = NONE;
	if (enctype == "WPA")
		res = WPA;
	else if (enctype == "WEP")
		res = WEP;

	return res;
}

void setWPA(const std::string& essid, const std::string& key, ENC enc)
{
	if (essid.empty() || (enc != NONE && key.empty()))
		return;

	std::ofstream wpa_conf_os("/etc/wpa_supplicant/wpa_supplicant.conf");
	wpa_conf_os << "network={\n";
	wpa_conf_os << "\tssid=\"" << essid << "\"\n";
	switch (enc)
	{
	case WPA:
		wpa_conf_os << "\tpsk=\"" << key << "\"\n";
		break;
	case WEP:
		wpa_conf_os << "\twep_key0=\"" << key << "\"\n";
		wpa_conf_os << "\twep_tx_keyidx=0\n";
		//continue
	case NONE:
		wpa_conf_os << "\tkey_mgmt=NONE\n";
		break;
	default:
		break;
	}
	wpa_conf_os << "}\n";
	wpa_conf_os.close();
}

void setAP()
{
	exec("/sbin/ifdown wlan0");
	exec("/usr/sbin/service hostapd stop");
	exec("/usr/sbin/service udhcpd stop");

    std::ofstream interfaces("/etc/network/interfaces");
    interfaces << "auto lo\n\n";
    interfaces << "iface lo inet loopback\n";
    interfaces << "iface eth0 inet dhcp\n\n";
    interfaces << "auto wlan0\n";
    interfaces << "iface wlan0 inet static\n";
    interfaces << "address 10.0.0.1\n";
    interfaces << "netmask 255.255.255.0\n\n";
    interfaces << "up iptables-restore < /etc/iptables.ipv4.nat\n";

    interfaces.close();

	exec("/sbin/ifup wlan0");
	exec("/usr/sbin/service hostapd start");
	exec("/usr/sbin/service udhcpd start");
}

void setCli()
{
	exec("/sbin/ifdown wlan0");
	exec("/usr/sbin/service hostapd stop");
	exec("/usr/sbin/service udhcpd stop");

    std::ofstream interfaces("/etc/network/interfaces");
    interfaces << "auto lo\n\n";
    interfaces << "iface lo inet loopback\n";
    interfaces << "iface eth0 inet dhcp\n\n";
    interfaces << "auto wlan0\n";
    interfaces << "iface wlan0 inet dhcp\n";
    interfaces << "wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf\n";
    interfaces << "iface default inet dhcp\n";

    interfaces.close();

	exec("/sbin/ifup wlan0");
}