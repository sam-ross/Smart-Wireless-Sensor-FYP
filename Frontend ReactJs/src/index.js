import React from 'react';
import ReactDOM from 'react-dom/client';
import './index.css';

class MainWrapper extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      // baseUrl: "http://192.168.0.26:8081/getReadings",
      baseUrl: "https://smart-wireless-sensor-backend.nw.r.appspot.com/getReadings",

      voltage: 0.0,
      current: 0.0,
      frequency: 0.0,
      power: 0.0,

      avgVoltage: 0.0,
      avgCurrent: 0.0,
      avgFrequency: 0.0,
      avgPower: 0.0,

      energyConsumption: 0.0,
      numberOfFaults: 0,
      timeElapsed: 0.0,
      offset: 0.0,

      demo: "demo-on",
      counter: 0,

      // alphabetical here for handy alignment with chrome dev tools for obtaining demo values
      demoAvgCurrent: [18.601, 18.599, 18.598, 18.596, 18.596],
      demoAvgFrequency: [49.707, 49.714, 49.701, 49.705, 49.717],
      demoAvgPower: [198.121, 198.119, 198.079, 198.032, 198.066],
      demoAvgVoltage: [10.651, 10.650, 10.648, 10.651, 10.654],
      demoCurrent: [18.598, 18.596, 18.596, 18.601, 18.599],
      demoFrequency: [49.701, 49.705, 49.717, 49.707, 49.714],
      demoOffset: [2.497, 2.483, 2.5, 2.484, 2.497],
      demoPower: [198.032, 198.066, 198.121, 198.119, 198.079],
      demoTimeElapsed: 1,
      demoVoltage: [10.648, 10.651, 10.654, 10.651, 10.650],

      prevResultTimeElapsed: 0.0,
      sameReadingsCounter: 0,
    };
  }

  componentDidMount = () => {
    this.myTimer = setInterval(() => {
      this.getReadings();
    }, 1000);
  }

  componentWillUnmount = () => {
    clearInterval(this.myTimer);
  }

  getReadings() {
    fetch(this.state.baseUrl)
      .then((res) => {
        if (res.status >= 400 || res.status === 204) {
          this.setDemoValues();
          throw new Error(res.status);
        }
        return res.json();
      })
      .then(
        (result) => {
          console.log(result);
          let demoResult = "demo-off";
          let updatedSameReadingsCounter = this.state.sameReadingsCounter + 1;
          if (result.timeElapsed !== this.state.prevResultTimeElapsed) {
            updatedSameReadingsCounter = 0;
          }

          this.setState({
            sameReadingsCounter: updatedSameReadingsCounter
          });

          if (this.state.sameReadingsCounter > 7) {
            this.setDemoValues();
            demoResult = "demo-on";
          } else {
            if (result.timeElapsed === this.state.prevResultTimeElapsed) {

            }
            this.setState({
              voltage: result.voltage,
              current: result.current,
              frequency: result.frequency,
              power: result.power,

              avgVoltage: result.avgVoltage,
              avgCurrent: result.avgCurrent,
              avgFrequency: result.avgFrequency,
              avgPower: result.avgPower,

              energyConsumption: result.energyConsumption,
              numberOfFaults: result.faultCounter,
              timeElapsed: result.timeElapsed,
              offset: result.offset,

              demoTimeElapsed: result.timeElapsed,

              demo: demoResult,

              prevResultTimeElapsed: result.timeElapsed,
            })
          }
        },
        (error) => {
          console.log("Unexpected error returned: " + error.message);
          this.setDemoValues();
        }
      )
  }

  setDemoValues() {
    let count = this.state.counter;
    this.setState({
      demoTimeElapsed: this.state.demoTimeElapsed + 1,
      demoEnergyConsumption: this.state.demoEnergyConsumption + 0.055023167,
    })

    // reset demo state every 24 hours
    if (this.state.demoTimeElapsed === 86401) {
      this.setState({
        demoTimeElapsed: 0,
        demoEnergyConsumption: 0,
      })
    }

    this.setState({
      voltage: this.state.demoVoltage[count % 5],
      current: this.state.demoCurrent[count % 5],
      frequency: this.state.demoFrequency[count % 5],
      power: this.state.demoPower[count % 5],

      avgVoltage: this.state.demoAvgVoltage[count % 5],
      avgCurrent: this.state.demoAvgCurrent[count % 5],
      avgFrequency: this.state.demoAvgFrequency[count % 5],
      avgPower: this.state.demoAvgPower[count % 5],

      energyConsumption: this.state.demoAvgPower[count % 5] * this.state.demoTimeElapsed / 3600.0,
      timeElapsed: this.state.demoTimeElapsed,
      offset: this.state.demoOffset[count % 5],
      counter: count + 1,

      demo: "demo-on"
    });
  }


  render() {
    return (
      <div className="container">
        <header className="header">
          <h1 className="header1">Smart Wireless Sensor Readings</h1>
        </header>

        <div className="section-middle">
          <div className='outer-row'>
            <div className="row">
              <div className="readings">
                <p>Voltage:</p>
              </div>
              <div className="readings">
                <p>Current:</p>
              </div>
              <div className="readings">
                <p>Frequency:</p>
              </div>
              <div className="readings">
                <p>Power:</p>
              </div>
            </div>

            <div className="row">
              <div className="readings">
                <p>{this.state.voltage.toFixed(3)}kV</p>
              </div>
              <div className="readings">
                <p>{this.state.current.toFixed(3)}A</p>
              </div>
              <div className="readings">
                <p>{this.state.frequency.toFixed(3)}Hz</p>
              </div>
              <div className="readings">
                <p>{this.state.power.toFixed(3)}kW</p>
              </div>
            </div>
          </div>

          <div className='outer-row'>
            <div className="row">
              <div className="readings">
                <p>Avg Voltage:</p>
              </div>
              <div className="readings">
                <p>Avg Current:</p>
              </div>
              <div className="readings">
                <p>Avg Frequency:</p>
              </div>
              <div className="readings">
                <p>Avg Power:</p>
              </div>
            </div>

            <div className="row">
              <div className="readings">
                <p>{this.state.avgVoltage.toFixed(3)}kV</p>
              </div>
              <div className="readings">
                <p>{this.state.avgCurrent.toFixed(3)}A</p>
              </div>
              <div className="readings">
                <p>{this.state.avgFrequency.toFixed(3)}Hz</p>
              </div>
              <div className="readings">
                <p>{this.state.avgPower.toFixed(3)}kW</p>
              </div>
            </div>
          </div>

          <div className='outer-row'>
            <div className="row">
              <div className="readings" id='readings'>
                <p>Energy Consumption:</p>
              </div>
              <div className="readings" id='readings'>
                <p>Number of Faults:</p>
              </div>
              <div className="readings" id='readings'>
                <p>Time Elapsed:</p>
              </div>
              <div className="readings" id='readings'>
                <p>Offset/midpoint:</p>
              </div>
            </div>

            <div className="row">
              <div className="readings" id='readings'>
                <p>{this.state.energyConsumption.toFixed(3)}kWh</p>
              </div>
              <div className="readings" id='readings'>
                <p>{this.state.numberOfFaults}</p>
              </div>
              <div className="readings" id='readings'>
                <p>{this.state.timeElapsed}s</p>
              </div>
              <div className="readings" id='readings'>
                <p>{this.state.offset.toFixed(3)}V</p>
              </div>
            </div>
          </div>
        </div>

        <header className="header">
          <div className={this.state.demo}>
            Demo on: ESP32 pipeline is currently not sending any data to the backend
          </div>
        </header>
      </div>
    );
  }
}

// ========================================

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(<MainWrapper />);

