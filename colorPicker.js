// Carduino Comzas Final Integration - thdoctor - Martin Robotics
class ACPT {
  constructor() {
    this.video = document.createElement('video');
    this.canvas = document.createElement('canvas');
    this.ctx = this.canvas.getContext('2d');
    this.securityHash = "COMZAS_SECURE_2024";
    this.servoAngle = 90;
    this.debugMode = true;
    
    // Camera Configuration
    this.FOCAL_LENGTH = 720;
    this.KNOWN_WIDTH = 30;
    this.MIN_CONTOUR_AREA = 150;
    
    // Color Detection
    this.colorThresholds = {
      red: { h: [0, 15], s: [100, 255], v: [50, 255] },
      yellow: { h: [25, 35], s: [100, 255], v: [50, 255] },
      green: { h: [35, 85], s: [100, 255], v: [50, 255] }
    };

    this.initDOM();
  }

  initDOM() {
    // Camera view will create a new camera view angle in the html 
    const camContainer = document.createElement('div');
    camContainer.style.position = 'fixed';
    camContainer.style.top = '20px';
    camContainer.style.right = '20px';
    camContainer.style.zIndex = '1000';
    this.canvas.width = 640;
    this.canvas.height = 480;
    camContainer.appendChild(this.canvas);
    document.body.appendChild(camContainer);

    // Sensor Map will create a line or something that will trace out to the us servo rotationm it will also create a new canvas
    this.sonarCanvas = document.createElement('canvas');
    this.sonarCanvas.width = 300;
    this.sonarCanvas.height = 300;
    this.sonarCanvas.style.position = 'fixed';
    this.sonarCanvas.style.top = '20px';
    this.sonarCanvas.style.left = '20px';
    this.sonarCtx = this.sonarCanvas.getContext('2d');
    this.sonarCtx.transform(1, 0, 0, -1, 0, this.sonarCanvas.height);
    document.body.appendChild(this.sonarCanvas);
  }

  async init() {
    await this.startCamera();
    this.initColorProcessing();
    this.drawLoop();
  }

  async startCamera() {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({
        video: { width: 640, height: 480, facingMode: 'environment' }
      });
      this.video.srcObject = stream;
      await this.video.play();
    } catch (error) {
      this.showError('CAMERA FAILED: ' + error.message);
    }
  }

  initColorProcessing() {
    this.kernel = {
      sharpen: [[0, -1, 0], [-1, 5, -1], [0, -1, 0]],
      edgeDetect: [[1, 0, -1], [0, 0, 0], [-1, 0, 1]]
    };
  }

  drawLoop() {
    if (this.video.readyState >= 2) {
      this.processFrame();
      this.drawSensorMap();
    }
    requestAnimationFrame(() => this.drawLoop());
  }

  processFrame() {
    this.ctx.drawImage(this.video, 0, 0);
    let frame = this.ctx.getImageData(0, 0, this.canvas.width, this.canvas.height);
    frame = this.applyKernel(frame, this.kernel.sharpen);
    const contours = this.detectColors(frame);
    this.makeDecision(contours);
    if(this.debugMode) this.drawDebugInfo(contours);
  }

  detectColors(frame) {
    const hsv = this.rgbToHSV(frame);
    const contours = [];
    
    Object.keys(this.colorThresholds).forEach(color => {
      const mask = this.createMask(hsv, color);
      const colorContours = this.findContours(mask);
      contours.push(...colorContours.filter(c => c.area > this.MIN_CONTOUR_AREA));
    });
    
    return contours;
  }

  createMask(hsvFrame, color) {
    const mask = new Uint8Array(hsvFrame.data.length / 4);
    const { h: [hMin, hMax], s: [sMin, sMax], v: [vMin, vMax] } = this.colorThresholds[color];
    
    for(let i = 0; i < hsvFrame.data.length; i += 4) {
      const [h, s, v] = hsvFrame.data.slice(i, i + 3);
      if(h >= hMin && h <= hMax && s >= sMin && s <= sMax && v >= vMin && v <= vMax) {
        mask[i/4] = 255;
      }
    }
    return mask;
  }

  makeDecision(contours) {
    const scores = contours.map(contour => ({
      color: contour.color,
      score: contour.area * (this.canvas.height - contour.centroid.y)
    }));
    
    const best = scores.reduce((a, b) => a.score > b.score ? a : b, {score: 0});
    if(best.score > 1000) this.sendCommand(best.color.toUpperCase());
  }

  drawSensorMap() {
    this.sonarCtx.clearRect(0, 0, this.sonarCanvas.width, this.sonarCanvas.height);
    this.sonarCtx.fillStyle = '#2196F3';
    this.sonarCtx.beginPath();
    this.sonarCtx.arc(150, 280, 10, 0, Math.PI*2);
    this.sonarCtx.fill();
  }

  updateSensor(distance, angle) {
    const radians = angle * (Math.PI/180);
    const x = 150 + (distance * Math.cos(radians))/2;
    const y = 280 - (distance * Math.sin(radians))/2;
    
    this.sonarCtx.fillStyle = `rgba(255, ${255 - distance}, 0, 0.8)`;
    this.sonarCtx.beginPath();
    this.sonarCtx.arc(x, y, 5, 0, Math.PI*2);
    this.sonarCtx.fill();
    
    this.drawSensorLine(angle, distance);
  }

  drawSensorLine(angle, distance) {
    this.ctx.beginPath();
    this.ctx.moveTo(this.canvas.width/2, this.canvas.height);
    this.ctx.lineTo(
      this.canvas.width/2 + Math.tan(angle * (Math.PI/180)) * 100,
      this.canvas.height - (distance * 2)
    );
    this.ctx.strokeStyle = '#ff0000';
    this.ctx.stroke();
  }

  sendCommand(command) {
    const payload = {
      cmd: command,
      t: Date.now(),
      hash: btoa(`${command}|${Date.now()}|${this.securityHash}`)
    };
    console.log('SENDING:', payload);
    if(window.serialPort) {
      const writer = window.serialPort.writable.getWriter();
      writer.write(new TextEncoder().encode(JSON.stringify(payload)));
      writer.releaseLock();
    }
  }

  showError(message) {
    const errorDiv = document.createElement('div');
    errorDiv.style.color = 'red';
    errorDiv.textContent = 'ERROR: ' + message;
    document.body.prepend(errorDiv);
  }
}

const carduinoAI = new ACPT();
carduinoAI.init();

function handleArduinoData(data) {
  if(data.startsWith('DIST:')) {
    const distance = parseFloat(data.split(':')[1]);
    carduinoAI.updateSensor(distance, carduinoAI.servoAngle);
  }
}