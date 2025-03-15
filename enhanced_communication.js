import { initializeApp } from 'firebase/app';
import { getDatabase, ref, set } from 'firebase/database';

const firebaseConfig = {
  apiKey: "your-api-key",
  authDomain: "your-project.firebaseapp.com",
  databaseURL: "https://your-project.firebaseio.com",
  projectId: "your-project",
  storageBucket: "your-project.appspot.com",
  messagingSenderId: "your-sender-id",
  appId: "your-app-id"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

export function sendCommand(command) {
  const commandRef = ref(db, 'devices/carduino01/command');
  set(commandRef, command)
    .then(() => console.log('Command sent:', command))
    .catch((error) => console.error('Send failed:', error));
}

export function emergencyOverride() {
  const overrideRef = ref(db, 'devices/carduino01/emergency');
  set(overrideRef, true)
    .then(() => console.log('Emergency activated'))
    .catch((error) => console.error('Override failed:', error));
}