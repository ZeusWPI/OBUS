/**
 * Soundeffects
 */
const sounds = {
	strike: new Audio("/static/sounds/strike.mp3"),
	alarm: new Audio("/static/sounds/alarm.mp3"),
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
			document.getElementById("gamestate").innerHTML = state.gamestate;

			// Reset the strike amount if the game is not running anymore.
			if (state.gamestate != "GAME") {
				state.strikeAmount = 0;
				state.alarmPlayed = false;
			}

			if (state.gamestate === "GAME") {
				// Update the estimated timeout date
				state.estimatedTimeout = new Date(
					new Date().getTime() + data.timeleft * 1000
				);

				// Calculate the ew total amount of strikes for all puzzles.
				const newStrikes = data.puzzles
					.map((p) => p.strikes)
					.reduce((a, b) => a + b, 0);

				// Play a "buzzer" sound when a strike is made.
				if (state.strikes < newStrikes) {
					playSound(sounds.strike);
				}

				// Play a "alarm" sound when the time is at 10 seconds.
				if (data.timeleft <= 20 && data.timeleft > 19 && !state.alarmPlayed) {
					playSound(sounds.alarm);
					state.alarmPlayed = true;
				}

				// Update the total amount of strikes
				state.strikes = newStrikes;
			}

			// Update the start/restart button visibility.
			const startButton = document.querySelector("#buttonStart");
			const restartButton = document.querySelector("#buttonRestart");
			startButton.disabled = state.gamestate !== "DISCOVER";
			restartButton.disabled = state.gamestate !== "GAMEOVER";

			// Update the modules
			updateModules(data.puzzles);
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

/**
 * Update the segment display with the latest game data.
 */
function updateSegmentDisplay() {
	// Do not update the timer when the game is not running.
	if (state.gamestate != "GAME" || !state.estimatedTimeout) {
		return;
	}

	const timeLeft = (state.estimatedTimeout - new Date()) / 1000;

	// Set the display to 0 when there is no time left.
	if (timeLeft <= 0) {
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
 * When the window is loaded.
 */
window.addEventListener("load", () => {
	// Initiliaz the game state.
	initializeGameState();

	// Initialize the segment display.
	initializeSegmentDisplay();
});
