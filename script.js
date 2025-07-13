var info = [];
function get_info() {
	var request = new XMLHttpRequest();
	request.open('GET', '/status', true);
	request.responseType = 'json';
	request.onload = function() {
		if (request.readyState == 4 && request.status == 200) {
			info = request.response;
			print_info();
		}
	}
	request.send();
}

function print_info() {
	document.getElementById("mode_val").textContent=info["Mode"];
	var tempElem = document.getElementById("temp_el");
	tempElem.textContent = "".concat(info["Temp"],info ["TempMode"]);
	btn_mode.value = info["Mode"];
	if (info["Mode"] == "AUTO") {
		btn_mode.value = info["Mode"];
		btn_mode.classList.add('On');
		btn_mode.classList.remove('Manual');
	} else {
		btn_mode.value = info["Mode"];
		btn_mode.classList.remove('On');
		btn_mode.classList.add('Manual');				
	}

	if (info["Pump"] == 1) {
		btn_irrigator.value = "On"
		btn_irrigator.classList.add('On');
		btn_irrigator.classList.remove('Off');
	} else {
		btn_irrigator.value = "Off"
		btn_irrigator.classList.remove('On');	
		btn_irrigator.classList.add('Off');			
	}

	if (info["Red"] == 0) {
		btn_red_led.value = "On"
		btn_red_led.classList.add('On');
		btn_red_led.classList.remove('Off');
	} else {
		btn_red_led.value = "Off"
		btn_red_led.classList.remove('On');	
		btn_red_led.classList.add('Off');			
	}
	if (info["Blue"] == 0) {
		btn_blue_led.value = "On"
		btn_blue_led.classList.add('On');
		btn_blue_led.classList.remove('Off');
	} else {
		btn_blue_led.value = "Off"
		btn_blue_led.classList.add('Off');
		btn_blue_led.classList.remove('On');				
	}
}

function change_mode() {
	var path = info["Mode"] == "AUTO" ? "/mode/MANUAL" : "/mode/AUTO";
	var request = new XMLHttpRequest();
	request.open('GET', path, false);
	request.onload = function() {
		if (request.readyState == 4 && request.status == 200) {
			var data = JSON.parse(request.responseText);
			btn_mode.value = data["Mode"];
			info["Mode"] = data["Mode"];
			txt_mode.textContent = data["Mode"];
			if (data["Mode"] == "AUTO") {
				btn_mode.classList.add('On');
				btn_mode.classList.remove('Manual');
			} else {
				btn_mode.classList.remove('On');
				btn_mode.classList.add('Manual');				
			}
		}
	}
	request.send();
}

function turn_irrigation() {
	var path = info["Pump"] == 0 ? "/waterOn" : "/waterOff";
	var request = new XMLHttpRequest();
	request.open('GET', path, false);
	request.onload = function() {
		if (request.readyState == 4 && request.status == 200) {
			var data = JSON.parse(request.responseText);
			info["Pump"] = data["Pump"];
			if (data["Pump"] == 1) {
				btn_irrigator.value = "On"
				btn_irrigator.classList.add('On');
				btn_irrigator.classList.remove('Off');
			} else {
				btn_irrigator.value = "Off"
				btn_irrigator.classList.remove('On');	
				btn_irrigator.classList.add('Off');			
			}
		}
	}
	request.send();
}

function turn_red_led() {
	var path = info["Red"] == 1 ? "/redOn" : "/redOff";
	var request = new XMLHttpRequest();
	request.open('GET', path, false);
	request.onload = function() {
		if (request.readyState == 4 && request.status == 200) {
			var data = JSON.parse(request.responseText);
			info["Red"] = data["Red"];
			if (data["Red"] == 0) {
				btn_red_led.value = "On"
				btn_red_led.classList.add('On');
				btn_red_led.classList.remove('Off');
			} else {
				btn_red_led.value = "Off"
				btn_red_led.classList.remove('On');	
				btn_red_led.classList.add('Off');			
			}
		}
	}
	request.send();
}

function turn_blue_led() {
	var path = info["Blue"] == 1 ? "/blueOn" : "/blueOff";
	var request = new XMLHttpRequest();
	request.open('GET', path, false);
	request.onload = function() {
		if (request.readyState == 4 && request.status == 200) {
			var data = JSON.parse(request.responseText);
			info["Blue"] = data["Blue"];
			if (data["Blue"] == 0) {
				btn_blue_led.value = "On"
				btn_blue_led.classList.add('On');
				btn_blue_led.classList.remove('Off');
			} else {
				btn_blue_led.value = "Off"
				btn_blue_led.classList.add('Off');
				btn_blue_led.classList.remove('On');				
			}
		}
	}
	request.send();
}

var txt_mode = document.getElementById("mode_val");
var btn_mode = document.getElementById("mode_changer_btn");
btn_mode.addEventListener('click', change_mode);
var btn_irrigator = document.getElementById("irrigator_btn");
btn_irrigator.addEventListener('click', turn_irrigation);
var btn_red_led = document.getElementById("red_led_btn");
btn_red_led.addEventListener('click', turn_red_led);
var btn_blue_led = document.getElementById("blue_led_btn");
btn_blue_led.addEventListener('click', turn_blue_led);

document.addEventListener('DOMContentLoaded', get_info);
setInterval(get_info, 10000);