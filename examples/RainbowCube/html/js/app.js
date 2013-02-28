(function () {
	window.config = {
		serverUrl: '[DeviceHive_api_url]',
		login: '[DeviceHive_login]',
		password: '[DeviceHive_Password]',
		deviceId: '[Device_Id]',
		cmdTimeout: 1000 * 60 * 2, //2 minutes
		ledCmdName: 'pixels',
		cubeSize: 4
	};


	var deviceHive = new DeviceHive(config.serverUrl, config.login, config.password),
	    currentTimeout;

	var deviceIdElem = $('.deviceId'),
		deviceNameElem = $('.device-name'),
	    deviceStatusElem = $('.device-status'),
		cmdInfoElem = $('.cmdInfo'),
	    colorPickerElem = $('#colorpicker'),
	    actionsElem = $('.actions');

	var fillQube = function (leftX, leftY, leftZ, width, height, depth, r, g, b) {
		if (!width || !height || !depth || leftX + width > config.cubeSize || leftY + depth > config.cubeSize || leftZ + height > config.cubeSize) {
			throw "Cannot fill the Cube.";
		}

		var result = [];

		for (var i = 0; i < leftX + width; i++) {
			for (var j = 0; j < leftY + depth; j++) {
				for (var k = 0; k < leftZ + height; k++) {
					result.push({
						X: '' + i,
						Y: '' + j,
						Z: '' + k,
						R: '' + r,
						G: '' + g,
						B: '' + b
					});
				}
			}
		}

		return result;
	};

	var handleError = function (xhr, textStatus, error) {
		cmdInfoElem.hide();
		if ($.type(xhr) === 'string') {
			$('.loading').text('Device is unplugged or not registered. Please register and plug-in your device and then reload this page.');
			return;
		}
		alert('An error occured: ' + textStatus + '. Error info: ' + JSON.stringify(error));
	};

	var sendCommand = function (deviceId, commandName, params, timeout) {
		var dfdResult = $.Deferred();

		deviceHive.sendCommand(deviceId, commandName, params).result(function (commandData) {
			var command = commandData;
			command.status === 'Failed'
				? dfdResult.reject(null, command.status, command)
				: dfdResult.resolve(command);
		});

		if (currentTimeout) {
			clearTimeout(currentTimeout);
		}

		currentTimeout = setTimeout(function () {
			dfdResult.rejectWith(null, [null, 'Timeout expired', 'timeout']);
		}, timeout || config.deviceHive.defaultNotificationTimeout);

		return dfdResult;
	};

	var buttonClickHandler = function (pos) {
		return function () {
			cmdInfoElem.fadeOut(function () {
				cmdInfoElem.text('Pending...');
				cmdInfoElem.fadeIn();

				var rgb = colorPickerElem.spectrum("get").toRgb();

				var cubeParams = fillQube(0, 0, 0, pos + 1, pos + 1, pos + 1, rgb.r, rgb.g, rgb.b);
				sendCommand(config.deviceId, config.ledCmdName, cubeParams, config.cmdTimeout).done(function (command) {
					cmdInfoElem.text(JSON.stringify(command.command.result));
				}).fail(handleError);

			});
		};
	};

	var createActions = function () {
		for (var i = 0; i < 4; i++) {
			var button = $('<input>').attr('id', 'actionButton' + i).attr('type', 'button').val(i + 1);
			button.click(buttonClickHandler(i));

			actionsElem.append(button);
		}
	};

	window.app = {
		init: function () {
			deviceIdElem.text('Device Id: ' + config.deviceId);
			return deviceHive.openChannel().then(function () {
				return deviceHive.getDevice(config.deviceId);
			}).then(function (device) {
				deviceNameElem.text(device.name);
				deviceStatusElem.text(device.status);
				
				colorPickerElem.spectrum({
					color: "#ECC",
					showInput: true,
					className: "colorpicker",
					preferredFormat: "hex",
					flat: true
				});

				createActions();

				return $('.loading').fadeOut().promise();

			}).then(function () {
				return $('.main').fadeIn().promise();
			}).fail(handleError);
		}
	};
})()