const fs = require('fs');
const fetch = require('node-fetch');

let tokens = [];

try {
  const file = fs.readFileSync('tokens.txt', 'utf8');
  tokens = file.split('\n').filter(token => token.trim() !== '');
} catch (err) {
  fs.writeFileSync('tokens.txt', '');
}

function getRandomToken() {
  if (tokens.length === 0) return null;
  const randomIndex = Math.floor(Math.random() * tokens.length);
  return tokens[randomIndex];
}

function setup() {
  const readline = require('readline').createInterface({
    input: process.stdin,
    output: process.stdout
  });

  return new Promise((resolve) => {
    readline.question('Enter your serverID below:\n', (serverId) => {
      readline.question('Enter your message below:\n', (msg) => {
        readline.close();
        resolve({ serverId, msg });
      });
    });
  });
}

async function mainLoop(serverId, msg) {
  while (true) {
    const token = getRandomToken();
    if (!token) {
      console.log('No tokens available. Exiting...');
      return;
    }

    const url = `https://discord.com/api/v8/channels/${serverId}/messages`;
    const payload = { content: msg };
    const headers = { Authorization: token, 'Content-Type': 'application/json' };

    try {
      const response = await fetch(url, {
        method: 'POST',
        headers: headers,
        body: JSON.stringify(payload)
      });
      console.log(response.status);

      if (response.status === 401) {
        tokens = tokens.filter(t => t !== token);
      } else {
        break;
      }
    } catch (error) {
      console.error(error);
      return;
    }

  
    await new Promise(resolve => setTimeout(resolve, 500));
  }
}

async function main() {
  const readline = require('readline');
  readline.emitKeypressEvents(process.stdin);
  process.stdin.setRawMode(true);

  setup().then(({ serverId, msg }) => {
    const numThreads = 60;
    const tasks = [];

    for (let i = 0; i < numThreads; i++) {
      tasks.push(mainLoop(serverId, msg));
    }

    Promise.all(tasks).then(() => {
      process.exit();
    });
  });
}

main();
