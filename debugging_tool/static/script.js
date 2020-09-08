
let maxseen = 0;
let paused = true;
let updaterID = null;

function updateShow() {
	for (let item of document.getElementsByClassName("raw")) {
		if (document.getElementById('show_raw').checked) {
			item.classList.remove("hide");
		} else {
			item.classList.add("hide");
		}
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
						let current = data[i];

						let human_readable_type = row.insertCell(0)
						human_readable_type.innerHTML = current['human_readable_type'];
						human_readable_type.className = 'human_readable_type';

						let sender_id = row.insertCell(-1)
						sender_id.innerHTML = current['sender_id'];
						sender_id.className = 'sender_id';

						let parsed = row.insertCell(-1)
						if (current['parsed'].startsWith("PARSE ERROR")) {
							parsed.setAttribute("error", true);
						}
						parsed.innerHTML = current['parsed'];
						parsed.className += 'parsed';

						let time = row.insertCell(-1)
						time.innerHTML = current['time'];
						time.className = 'time';

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

