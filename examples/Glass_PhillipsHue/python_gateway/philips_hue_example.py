#!/usr/bin/env python
# -*- coding: utf8 -*-
# vim:set et tabstop=4 shiftwidth=4 nu nowrap fileencoding=utf-8:

import sys
import os
import argparse
import urllib2
import json

from twisted.python import log
from twisted.internet import reactor


sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import devicehive.client
import devicehive.auto
import devicehive.interfaces
import devicehive.poll
import devicehive.gateway
import devicehive.gateway.binary
from devicehive import DeviceInfo, Equipment, DeviceClass


class BridgeConnector(object):
    def __init__(self, ip):
        self.ip = ip

    def register_developer(self):
        """
        may be errors with this, philips hue documentation is not clear
        http://developers.meethue.com/gettingstarted.html
        """
        opener = urllib2.build_opener(urllib2.HTTPHandler)
        data = {
            'devicetype': 'devicehive_gateway user',
            'username': 'devicehive_gateway'
        }
        request = urllib2.Request(
            url='{0}/api'.format(self.ip),
            data=json.dumps(data),
        )
        request.add_header('Content-Type', 'application/json')
        request.get_method = lambda: 'POST'
        response = json.loads(opener.open(request).read())
        success = response[0].get('success')
        if success:
            self.username = success.get('username')
            return True
        else:
            return False

    def get_lamps(self):
        lamps_state = []
        for lamp_id in self.get_lamp_list():
            lamps_state.append(
                self.get_lamp_state(lamp_id)
            )
        if False in lamps_state:
            return False
        return lamps_state

    def get_lamp_list(self):
        opener = urllib2.build_opener(urllib2.HTTPHandler)
        request = urllib2.Request(
            url='{0}/api/{1}/lights'.format(self.ip, self.username)
        )
        response = json.loads(opener.open(request).read())
        return response.keys()

    def get_lamp_state(self, lamp_id):
        opener = urllib2.build_opener(urllib2.HTTPHandler)
        request = urllib2.Request(
            url='{0}/api/{1}/lights/{2}'.format(self.ip, self.username, lamp_id)
        )
        response_raw = json.loads(opener.open(request).read())
        if response_raw.get('error'):
            return False
        return {
            'id': lamp_id,
            'on': response_raw['state']['on'],
            'hue': response_raw['state']['hue'],
            'bri': response_raw['state']['bri'],
            'sat': response_raw['state']['sat'],
        }

    def set_lamps_state(self, lamps_states):
        result = []
        for lamp_state in lamps_states:
            lamp_id = lamp_state['id']
            del lamp_state['id']
            opener = urllib2.build_opener(urllib2.HTTPHandler)
            request = urllib2.Request(
                url='{0}/api/{1}/lights/{2}/state'.format(self.ip, self.username, lamp_id),
                data=json.dumps(lamp_state),
            )
            request.add_header('Content-Type', 'application/json')
            request.get_method = lambda: 'PUT'
            response = json.loads(opener.open(request).read())
            if type(response) == dict and response.get('error'):
                result.append(False)
            else:
                result.append(True)
        if False not in result:
            return {'success': True}
        else:
            return {'success': False}


DEVICE_INFO = DeviceInfo(
    id='8213426c-ecba-4644-9455-1d02be3c4dca',
    key='6e419dd8-1fcd-45a5-9365-bc498ac81caa',
    name='philips hue DEV',
    equipment=[Equipment(name='philips HUE equip', code='555', type='lamp'), ],
    device_class=DeviceClass(name='PhilipsHue', version='0.1')
)


class PhilipsHueGateway(devicehive.gateway.BaseGateway):
    devices = {
        'main': DEVICE_INFO
    }

    def __init__(self, url, factory_cls, bridge_ip):
        self.bridge_connector = BridgeConnector(bridge_ip)
        super(PhilipsHueGateway, self).__init__(url, factory_cls)

    def registration_received(self, device_info):
        print 'Registration received from device.'
        transport = devicehive.client.ws.WebSocketFactory()
        transport.connect('http://nn2015.pg.devicehive.com/api')
        super(PhilipsHueGateway, self).registration_received(device_info)

    def notification_received(self, device_info, notification):
        super(PhilipsHueGateway, self).notification_received(device_info, notification)

    def do_command(self, sender, command, finish_deferred):
        if command.command == 'getLamps':
            result = self.bridge_connector.get_lamps()
            if result:
                finish_deferred.callback(devicehive.CommandResult('Completed', result))
            else:
                self.factory.notify('errorNotification', {'errorId': 3, 'txt': "can't get lamp list"})

        elif command.command == 'setLamps':
            result = self.bridge_connector.set_lamps_state(command.parameters)
            if result:
                finish_deferred.callback(devicehive.CommandResult('Completed', result))
            else:
                self.factory.notify('errorNotification', {'errorId': 2, 'txt': "can't set params"})

    def on_connected(self):
        self.connected = True
        for key in self.devices:
            self.connect_device(self.devices[key])
        if self.bridge_connector.register_developer():
            self.factory.notify('connectionInfo', {'success': True})
        else:
            self.factory.notify('errorNotification', {'errorId': 1, 'txt': "can't connect to Phillips hue"},)

    def run(self, device_factory):
        self.device_factory = device_factory


def main(bridge_ip):
    log.startLogging(sys.stdout)
    gateway = PhilipsHueGateway('http://nn2015.pg.devicehive.com/api', devicehive.auto.AutoFactory, bridge_ip)
    #factory to be used to organize communication channel to device
    bin_factory = devicehive.gateway.binary.BinaryFactory(gateway)
    # run gateway application
    gateway.run(bin_factory)
    reactor.run()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    #for
    parser.add_argument('-i', '--ip', type=str, default='http://127.0.0.1:5000', dest='bridge_id', required=False, help='bridge ip')
    r = parser.parse_args()
    main(r.bridge_id)


