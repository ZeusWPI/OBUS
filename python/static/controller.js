let estimated_timeout_date;
let display;
let gamestate;
let last_strike_amount = 0;
let strike_audio = new Audio('/static/strike.mp3');
strike_audio.preload = 'auto';
let last_puzzle_state = []

function startbutton() {
	fetch('/start');
}

function restartbutton() {
	fetch('/restart');
}

function updateDisplay() {
	if (gamestate != 'GAME' || !estimated_timeout_date) {
		return;
	}
	setTimeleft((estimated_timeout_date - (new Date()))/1000);
}

function updateModuleState(data) {
	if (data.gamestate == 'INACTIVE' || data.gamestate == 'INFO') {
		document.getElementById("modules").innerHTML = '';
	}
	else {
		let newmodulestate = [];
		for (const puzzlestate of data.puzzles) {
			let modulediv = document.createElement('div');
			modulediv.textContent = puzzlestate.address;
			if (puzzlestate.solved === true) {
				modulediv.className = "solved";
			} else if (puzzlestate.solved === false) {
				modulediv.className = "unsolved";
			} else if (puzzlestate.solved === null) {
				modulediv.className = "needy";
			}
			newmodulestate.push(modulediv);
		}
		document.getElementById("modules").replaceChildren(...newmodulestate);
	}
}

function setTimeleft(timeleft) {
	if (timeleft <= 0) {
		display.setValue('00:00.0');
		return
	}
	let integral = Math.floor(timeleft);
	let fractional = timeleft - integral;
	let minutes = Math.floor(integral / 60);
	let seconds = integral % 60;
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
				gamestate = data.gamestate;
				document.getElementById('gamestate').innerHTML = gamestate;
				if (gamestate != 'GAME') {
					last_strike_amount = 0;
				}
				else if (gamestate == 'GAME') {
					// TODO maybe smooth this with the previous value of estimated_timeout_date?
					let new_estimate = new Date();
					new_estimate.setTime(new_estimate.getTime() + data.timeleft*1000);
					estimated_timeout_date = new_estimate;
					let total_strikes = data.puzzles.map(x => x.strikes).reduce((a, b) => a + b, 0);
					if (last_strike_amount < total_strikes) {
						strike_audio.load();
						strike_audio.play();
					}
					last_strike_amount = total_strikes;
				}
				updateModuleState(data);
			});
		}
	)
}

window.onload = function() {
	setInterval(state_update, 500);
	display = new SegmentDisplay("display");
	display.pattern         = "##:##.#";
	display.displayAngle    = 0;
	display.digitHeight     = 100;
	display.digitWidth      = display.digitHeight / 2;
	display.digitDistance   = display.digitHeight / 10;
	display.segmentWidth    = display.digitHeight / 10;
	display.segmentDistance = display.digitHeight / 100;
	display.segmentCount    = 7;
	display.cornerType      = 1;
	display.colorOn         = "#24dd22";
	display.colorOff        = "#1b4105";
	display.draw();
	display.draw();
	setInterval(updateDisplay, 100);
};
