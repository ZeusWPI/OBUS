// Keep this the same as on the server!
let max_messages = 200;

let paused = true;
let updaterID = null;

let serverID = "";
let newest_message_index = -1;
let messageTable = document.getElementById('message_table');
let header = document.getElementById("table_header").cloneNode(true);

let color_classes = {
	"RESERVED TYPE": "error",
	"controller": "controller",
	"puzzle": "puzzle",
	"needy": "needy",
}

function updateShowRaw() {
	document.getElementById("message_table").classList.toggle("hide_raw", !document.getElementById('show_raw').checked);
}

function updateShowStates() {
	document.getElementById("message_table").classList.toggle("hide_consecutive_states", !document.getElementById('show_consecutive_states').checked);
}

function updateMessages() {
	fetch(`/${newest_message_index}/api.json`)
	.then(
		function(response) {
			if (response.status !== 200) {
				console.log('FAIL: ' + response.status);
				return;
			}
			response.json().then(function(data) {
				if (serverID !== data.server_id) {
					messageTable.textContent = '';
					messageTable.append(header);
					serverID = data.server_id;
					newest_message_index = -1;
				}

				let new_messages = data.newest_msg - newest_message_index;
				if (new_messages > 0) {
					for (let message of data.messages) {
						let row = messageTable.insertRow(1);
						row.classList.add("fade");
						let current = message;

						let human_readable_type = row.insertCell(0)
						let colorblock = document.createElement("div");
						colorblock.classList.add("colorblock");
						human_readable_type.append(colorblock);

						human_readable_type.innerHTML += current['human_readable_type'];

						human_readable_type.classList.add(color_classes[current['human_readable_type']]);
						human_readable_type.classList.add('human_readable_type');

						let sender_id = row.insertCell(-1)
						sender_id.innerHTML = current['sender_id'];
						sender_id.classList.add('sender_id');
						let parsed = row.insertCell(-1)
						if (current['parsed'].startsWith("PARSE ERROR") || current['parsed'].startsWith("CAN ERROR")) {
							parsed.classList.add("error");
						} else if (current['parsed'].startsWith("STATE")) {
							row.classList.add("staterow");
						}
						parsed.innerHTML = current['parsed'];
						parsed.classList.add('parsed');

						let time = row.insertCell(-1)
						time.innerHTML = current['time'];
						time.classList.add('time');

						let raw_message = row.insertCell(-1);
						raw_message.innerHTML = current['raw_message'];
						raw_message.classList.add("raw");
						raw_message.classList.add("raw_message");

						let raw_id = row.insertCell(-1);
						raw_id.innerHTML = current['pretty_raw_sender_id'];
						raw_id.classList.add("raw");
						raw_id.classList.add("raw_id");
					}
				}

				// delete children if there are too many :O
				// -1 so we don't delete the header :)
				while (messageTable.children.length - 1 > max_messages) {
					messageTable.removeChild(messageTable.lastChild);
				}
				newest_message_index = data.newest_msg;

			});
		}
	)
}

function toggle_logging() {
	if (paused) {
		paused = false;
		document.getElementById("toggle_button").innerHTML = "Pause";
		updaterID = setInterval(updateMessages, 1000);
	} else {
		paused = true;
		document.getElementById("toggle_button").innerHTML = "Start";
		clearInterval(updaterID);
	}
}

function clear_log() {
	while (messageTable.children.length > 1) {
		messageTable.removeChild(messageTable.lastChild);
	}
}

window.onload = toggle_logging;
