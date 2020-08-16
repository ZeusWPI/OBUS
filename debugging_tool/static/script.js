maxseen = 0;

function updateShow() {
	if (document.getElementById('show_raw').checked) {
		document.getElementById('messages').classList = '';
	} else {
		document.getElementById('messages').classList = 'hide_details';
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
				console.log(data);
				if (data.length > maxseen) {
						var messageContainer = document.getElementById('messages');
						for (let i = maxseen; i < data.length; i++) {
							var current = data[i];
							var time = document.createElement("p");
							time.innerHTML = current['time'];
							time.className = 'time';

							var parsed = document.createElement("p");
							parsed.innerHTML = current['parsed'];
							parsed.className = 'parsed';

							var sender_id = document.createElement("p");
							sender_id.innerHTML = current['sender_id'];
							sender_id.className = 'sender_id';

							var pretty_raw_sender_id = document.createElement("p");
							pretty_raw_sender_id.innerHTML = current['pretty_raw_sender_id'];
							pretty_raw_sender_id.className = 'pretty_raw_sender_id';

							var raw_message = document.createElement("p");
							raw_message.innerHTML = current['raw_message'];
							raw_message.className = 'raw_message';

							var human_readable_type = document.createElement("p");
							human_readable_type.innerHTML = current['human_readable_type'];
							human_readable_type.className = 'human_readable_type';

							var newNode = document.createElement("div");
							newNode.className = "message";
							newNode.append(time, parsed, sender_id, pretty_raw_sender_id, raw_message, human_readable_type);
							messageContainer.prepend(newNode)
						}
						maxseen = data.length;
				}
			});
		}
	)
}


window.onload = function() {
	updateShow()
	console.log("loaded");
	updateMessages();


	setInterval(function() {
		if (document.getElementById('pause').checked) {
			return;
		}
		updateMessages()

	}, 1000);
};
