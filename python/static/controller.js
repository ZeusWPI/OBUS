var estimated_timeout_date;
var display;
var laststate;

function startbutton() {
	fetch('/start');
}

function restartbutton() {
	fetch('/restart');
}

function updateDisplay() {
	if (laststate != 'GAME' || !estimated_timeout_date) {
		return;
	}

	setTimeleft((estimated_timeout_date - (new Date())) * 1000);
}

function setTimeleft(timeleft) {
	var integral = Math.floor(timeleft);
	var fractional = timeleft - integral;
	var minutes = Math.floor(integral / 60);
	var seconds = integral % 60;
	display.setValue(String(minutes).padStart(2, '0') + ':' + String(seconds).padStart(2, '0') + '.' + String(Math.floor(fractional * 10)));
}

function state_update() {
	// TODO automatically timeout this request after the update interval
	fetch('/status.json')
	.then(
		function(response) {
			if (response.status !== 200) {
				console.log('FAIL: ' + response.status);
				return;
			}
			response.json().then(function(data) {
				document.getElementById('gamestate').innerHTML = data['gamestate'];
				laststate = data['gamestate'];
				if (data['gamestate'] == 'GAME') {
					// TODO maybe smooth this with the previous value of estimated_timeout_date?
					var new_estimate = new Date();
					new_estimate.setSeconds(new_estimate.getSeconds() + data['timeleft']);
					estimated_timeout_date = new_estimate;
				}
			});
		}
	)
}

window.onload = function() {
	setInterval(state_update, 500);
	display = new SegmentDisplay("display");
	display.pattern         = "##:##.#";
	display.displayAngle    = 0;
	display.digitHeight     = 26.5;
	display.digitWidth      = display.digitHeight / 2;
	display.digitDistance   = 2.9;
	display.segmentWidth    = 3;
	display.segmentDistance = 0.3;
	display.segmentCount    = 7;
	display.cornerType      = 1;
	display.colorOn         = "#24dd22";
	display.colorOff        = "#1b4105";
	display.draw();
	display.draw();
};
