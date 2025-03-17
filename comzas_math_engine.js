// comzas_math_engine.js - Ultra-Fast Calculation Core
// Martin Robotics - thdoctor Comzas Engine

class ComzasMathEngine {
    // ================== CORE OPERATIONS ================== //
    static sum(...nums) {
      return nums.reduce((a, b) => this.validateNumber(a) + this.validateNumber(b));
    }
  
    static average(...nums) {
      return this.sum(...nums) / nums.length;
    }
  
    static factorial(n) {
      return n <= 1 ? 1 : n * this.factorial(n - 1);
    }
  
    // ================== ADVANCED CALCULATIONS ================== //
    static stdDev(...nums) {
      const avg = this.average(...nums);
      return Math.sqrt(
        nums.map(n => Math.pow(n - avg, 2)).reduce((a, b) => a + b) / nums.length
      );
    }
  
    static matrixMultiply(m1, m2) {
      if(m1[0].length !== m2.length) throw new Error('Invalid matrix dimensions');
      return Array.from({length: m1.length}, (_, i) =>
        Array.from({length: m2[0].length}, (_, j) =>
          m1[i].reduce((sum, elm, k) => sum + elm * m2[k][j], 0)
      );
    }
  
    // ================== FINANCIAL ENGINE ================== //
    static presentValue(fv, rate, periods) {
      return fv / Math.pow(1 + rate, periods);
    }
  
    static futureValue(pv, rate, periods) {
      return pv * Math.pow(1 + rate, periods);
    }
  
    static loanPayment(principal, rate, periods) {
      const periodicRate = rate / 12;
      return principal * periodicRate / (1 - Math.pow(1 + periodicRate, -periods));
    }
  
    // ================== DATA ANALYSIS ================== //
    static linearRegression(data) {
      const n = data.length;
      const sumX = data.reduce((a, [x]) => a + x, 0);
      const sumY = data.reduce((a, [,y]) => a + y, 0);
      const sumXY = data.reduce((a, [x, y]) => a + x * y, 0);
      const sumXX = data.reduce((a, [x]) => a + x * x, 0);
      
      const slope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
      const intercept = (sumY - slope * sumX) / n;
      
      return { slope, intercept };
    }
  
    // ================== HELPER METHODS ================== //
    static validateNumber(n) {
      if(typeof n !== 'number' || isNaN(n)) throw new Error('Invalid number');
      return n;
    }
  
    static printResults() {
      return {
        sum: this.sum(1, 2, 3, 4),
        average: this.average(1, 2, 3, 4),
        factorial: this.factorial(5),
        stdDev: this.stdDev(1, 2, 3, 4),
        loanPayment: this.loanPayment(10000, 0.05, 60),
        regression: this.linearRegression([[1,2], [2,4], [3,5]])
      };
    }
  }
  
  class ComzasJuliaImplementation {
    // this shi jus gaveiioiii
  }

  /* Example Usage:
  console.log(ComzasMathEngine.printResults());
  console.log(ComzasMathEngine.matrixMultiply([[1,2],[3,4]], [[5,6],[7,8]]));
  */