/*

This is a code snippet for how to use the time remaining on the timer in your puzzle module.

To avoid the time left getting stale and inaccurate, we instead calculate when the timer will run
out and store this deadline. When we need to get the time left, we can then calculate it.

This does not handle some edgecases:
- times and timedeltas are stored in a 32-bit unsigned int, this can overflow after ~50 days, causing all sorts of weirdness
 => avoid having modules powered on for a long time
- the get_millis_remaining() getting called before receiving the first state update
 => try to only call this function after human input
 => alternatively, keep track of whether you can call the `get_millis_remaining` function already in a boolean
*/

uint32_t deadline;


void callback_state(uint32_t time_left, uint8_t strikes, uint8_t max_strikes, uint8_t puzzle_modules_left) {
	// Mark arguments as not used
	(void)strikes;
	(void)max_strikes;
	(void)puzzle_modules_left;

	deadline = millis() + time_left;
}

uint32_t get_millis_remaining() {
	return deadline - millis();
}

void loop() {
	/* ... */
	bool is_message_valid = obus_module::loopPuzzle(&message, callback_game_start, callback_game_stop, empty_callback_info, callback_state);
	/* ... */
}
