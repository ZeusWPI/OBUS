let maxseen = 0;
let paused = true;
let updaterID = null;

let color_classes = {
	"RESERVED TYPE": "error",
	"controller": "controller",
	"puzzle": "puzzle",
	"needy": "needy",
}

function updateShow() {
	for (let item of document.getElementsByClassName("raw")) {
		item.classList.toggle("hide", !document.getElementById('show_raw').checked);
	}
}

function updateMessages() {
	fetch('/api.json')
	.then(
		function(response) {
			if (response.status !== 200) {
				console.log('FAIL: ' + response.status);
				return;
			}
			response.json().then(function(data) {
				if (data.length > maxseen) {
					let messageTable = document.getElementById('message_table');

					for (let i = maxseen; i < data.length; i++) {
						let row = messageTable.insertRow(1);
						row.classList.add("fade");
						let current = data[i];

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
						if (current['parsed'].startsWith("PARSE ERROR")) {
							parsed.classList.add("error");
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
					maxseen = data.length;
				}
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
