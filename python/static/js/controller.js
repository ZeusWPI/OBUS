/**
 * Soundeffects
 */
const sounds = {
	strike: new Audio("/static/sounds/strike.mp3"),
	alarm: new Audio("/static/sounds/alarm.mp3"),
	explosion: new Audio("/static/sounds/explosion.mp3"),
	victory: new Audio("/static/sounds/victory.mp3"),
};

/**
 * State
 */
const state = {
	// Segment display used for telling the remaining time.
	display: null,

	// Current game state.
	gamestate: null,

	// Estimated timeout date when the timer should be stopped.
	estimatedTimeout: new Date(),

	// Amount of strikes (wrong answers) the player has made.
	strikes: 0,

	// If the alarm has been played already
	alarmPlayed: false,

	// If the end-of-game animation/sound has been started already
	endOfGameAnimationPlayed: false,

	// Number of mistakes allowed without exploding the bomb
	maxAllowedStrikes: 0,

	// Time on the timer at the beginning of a game
	gameDuration: 0
};

/**
 * Play a given audio file
 * @param {Audio} sound Audio object containing a sound.
 */
function playSound(sound) {
	sound.load();
	sound.play();
}

/**
 * Update the modules.
 * @param {Array} List of puzzles
 */
function updateModules(puzzles) {
	// Delete the modules when the game is inactive or in info mode.
	if (state.gamestate == "INACTIVE" || state.gamestate == "INFO") {
		document.getElementById("modules").innerHTML = "";
		return;
	}

	const modulesElement = document.getElementById("modules");

	// Update the modules.
	const modules = [];
	for (const puzzle of puzzles) {
		const moduleElement = document.createElement("div");
		moduleElement.textContent = puzzle.address;

		// State: Solved
		if (puzzle.solved === true) {
			moduleElement.className = "solved";
		}

		// State: Unsolved
		if (puzzle.solved === false) {
			moduleElement.className = "unsolved";
		}

		// State: Needy
		if (puzzle.solved === null) {
			moduleElement.className = "needy";
		}

		modules.push(moduleElement);
	}

	modulesElement.replaceChildren(...modules);
}

function updateStrikes() {
	const indicators = [];
	for (let idx = 0; idx < state.maxAllowedStrikes; idx++) {
		let indicator = document.createElement("span");
		indicator.classList.add('strikeIndicator');
		indicator.textContent = '!';
		if (idx < state.strikes) {
			indicator.classList.add('active');
		} else {
			indicator.classList.add('inactive');
		}
		indicators.push(indicator);
	}
	document.getElementById("strikeIndicatorContainer").replaceChildren(...indicators);
}

/**
 * Initialize the game state.
 */
function initializeGameState() {
	updateGameState();
	setInterval(updateGameState, 500);
}

/**
 * Update the game state.
 */
function updateGameState() {
	fetch("/status.json")
		.then((res) => res.json())
		.then((data) => {
			// Update the game state
			state.gamestate = data.gamestate;
			state.gameDuration = data.game_duration;
			state.maxAllowedStrikes = data.max_allowed_strikes;
			document.getElementById("gamestate").innerHTML = state.gamestate;

			// Reset the strike amount if the game is not running anymore.
			if (state.gamestate === "INFO") {
				state.strikes = 0;
				state.alarmPlayed = false;
				state.endOfGameAnimationPlayed = false;
			}

			if (state.gamestate === "GAME") {
				// Update the estimated timeout date
				state.estimatedTimeout = new Date(
					new Date().getTime() + data.timeleft * 1000
				);

				// Calculate the total amount of strikes for all puzzles.
				const newStrikes = data.puzzles
					.map((p) => p.strikes)
					.reduce((a, b) => a + b, 0);

				// Play a "buzzer" sound when a strike is made and update total amount of strikes.
				if (state.strikes < newStrikes) {
					playSound(sounds.strike);
					state.strikes = newStrikes;
				}

				// Play a "alarm" sound when the time is at 20 seconds.
				if (data.timeleft <= 20 && data.timeleft > 19 && !state.alarmPlayed) {
					playSound(sounds.alarm);
					state.alarmPlayed = true;
				}
			} else if (state.gamestate == "GAMEOVER") {
				if (!state.endOfGameAnimationPlayed) {
					switch (data.cause) {
						case 'STRIKEOUT':
						case 'TIMEOUT':
							document.getElementById("explosion-video").play();
							playSound(sounds.explosion);
							for (const elem of document.getElementsByClassName('hide-on-explosion')) {
								elem.style.display = 'none';
							}
							setTimeout(() => {
								// Make UI visible again after animation has stopped playing
								for (const elem of document.getElementsByClassName('hide-on-explosion')) {
									elem.style.display = '';
								}
							}, 4000);
							break;
						case 'VICTORY':
							playSound(sounds.victory);
							break;
					}
					state.endOfGameAnimationPlayed = true;
				}
			}

			// Update the start/restart button visibility.
			const startButton = document.querySelector("#buttonStart");
			const restartButton = document.querySelector("#buttonRestart");
			const stopButton = document.querySelector("#buttonStop");
			startButton.disabled = state.gamestate !== "DISCOVER";
			restartButton.disabled = state.gamestate !== "GAMEOVER";
			stopButton.disabled = state.gamestate !== "GAME";

			// Update the modules
			updateModules(data.puzzles);
			updateStrikes();
		});
}

/**
 * When the start button is clicked.
 */
function onStartButtonClick() {
	fetch("/start");
}

/**
 * When the restart button is clicked.
 */
function onRestartButtonClick() {
	fetch("/restart");
}

/**
 * When the stop button is clicked.
 */
 function onStopButtonClick() {
	fetch("/stop");
}

/**
 * Initialize the segment display.
 */
function initializeSegmentDisplay() {
	display = new SegmentDisplay("display");
	display.pattern = "##:##.#";
	display.displayAngle = 0;
	display.digitHeight = 100;
	display.digitWidth = display.digitHeight / 2;
	display.digitDistance = display.digitHeight / 10;
	display.segmentWidth = display.digitHeight / 10;
	display.segmentDistance = display.digitHeight / 100;
	display.segmentCount = 7;
	display.cornerType = 1;
	display.colorOn = "#ff0000";
	display.colorOff = "#ff000022";
	display.draw();

	// Update the segment
	setInterval(updateSegmentDisplay, 100);
}

function setTimeleft(timeLeft) {
	// Set the display to 0 when there is no time left.
	if (isNaN(timeLeft) || timeLeft <= 0) {
		display.setValue("00:00.0");
		return;
	}
	const integral = Math.floor(timeLeft);
	const fractional = timeLeft - integral;
	const minutes = Math.floor(integral / 60);
	const seconds = integral % 60;

	const minutesStr = String(minutes).padStart(2, "0");
	const secondsStr = String(seconds).padStart(2, "0");
	const fractionalStr = String(Math.floor(fractional * 10)).padStart(1, "0");

	display.setValue(`${minutesStr}:${secondsStr}.${fractionalStr}`);
}

/**
 * Update the segment display with the latest game data.
 */
function updateSegmentDisplay() {
	if (state.gamestate === "INACTIVE" || state.gamestate === "INFO" || state.gamestate === "DISCOVER") {
		setTimeleft(state.gameDuration);
	}
	// Do not update the timer when the game is not running.
	else if (state.gamestate != "GAME" || !state.estimatedTimeout) {
		return;
	} else {
		setTimeleft((state.estimatedTimeout - new Date()) / 1000);
	}
}

/**
 * When the window is loaded.
 */
window.addEventListener("load", () => {
	initializeGameState();
	initializeSegmentDisplay();
});
