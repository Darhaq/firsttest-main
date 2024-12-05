const channelID = 2771658; // Your ThingSpeak Channel ID
const readAPIKey = 'W9RF5JQJ6UKWZXVN'; // Replace with your ThingSpeak Read API Key

// Set the ThingSpeak API endpoint to fetch the data
const url = `https://api.thingspeak.com/channels/${channelID}/fields/1.json?api_key=${readAPIKey}&results=1`;

async function fetchData() {
  try {
    const response = await fetch(url);
    const data = await response.json();

    // Extract the most recent values for the counters
    const field1 = data.feeds[0].field1 || 0; // counterIn
    const field2 = data.feeds[0].field2 || 0; // counterOut
    const field3 = data.feeds[0].field3 || 0; // finalCount
    const field4 = data.feeds[0].field4 || 0; // currentInRoom

    // Update the HTML elements with the new data
    document.getElementById('counterIn').textContent = field1;
    document.getElementById('counterOut').textContent = field2;
    document.getElementById('currentInRoom').textContent = field4;
    document.getElementById('finalCount').textContent = field3;
  } catch (error) {
    console.error('Error fetching data from ThingSpeak:', error);
  }
}

// Fetch data every 21 seconds (to match the ThingSpeak update interval)
setInterval(fetchData, 21000);
