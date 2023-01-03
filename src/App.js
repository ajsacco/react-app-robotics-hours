import './App.css';
import React, {useState} from 'react'
import Axios from 'axios'

import LoginForm from './components/LoginForm';
import FinalForm from './components/FinalForm';
import ForgotPassword from './components/ForgotPassword';

import { API_ENDPOINT } from './API';

function App() {
  const [user, setUser] = useState({name:"", token:"", level:""})
  const [error, setError] = useState("");
  const [signedIn, setSignIn] = useState("");
  const [totalTime, setTime] = useState("");
  const [status, setStatus] = useState("")
  const [forgot, setForgot] = useState(false)
  const [changePasswordSubmitLabel, setChangePasswordSubmitLabel] = useState("SUBMIT")
  const [showCPWSuccessLabel, setShowCPWSuccessLabel] = useState(false)
  const [signinLabel, setSigninLabel] = useState("Sign In")

  const Login = details => {  
    setSigninLabel("Please Wait...");

    apiRequest(Axios.get, {type: "signin", name: details.name, pw: details.password}).then((response) => {
      setSigninLabel("Sign In")
      if(response.data !== false){
        console.log(response.data)

        if(response.data["name"] === details.name){
          setError("")
          setUser({
            name: response.data["name"],
            token: response.data["token"],
            level: response.data["level"]
          })
        } else {
          setError(response.data["error"])
        }
      } else {
        setError("API Error")
      }
    })
  }

  const Logout = () => {
    apiRequest(Axios.get, {type: "signout", name: user.name}).then((response) => {
      setSignIn("")
      setTime("")
      setError("")
      setStatus("")
      setForgot(false)
      setUser({name: "", token : "", level: ""})
    })    
  }

  const requestName = () => {
    setSignIn("Logging activity...")
    apiRequest(Axios.post, {type: "logname", name: user.name}).then((response) =>{
      if (response.data["status"] === null) setSignIn("Error: " + response.data["error"])
      else setSignIn("You've logged " + response.data["status"] + "!")
    })
  }

  const getTotalTime = () => {
    getTotalTimeAdmin(user.name);
  }

  const adminRequestName = name => {
    if (name === "") name = user.name;

    setStatus("Loading...")
    apiRequest(Axios.post, {type: "logname", name: name}).then((response) => {
      if (response.data["status"] === undefined) setStatus("Error: " + response.data["error"])
      else setStatus("You've logged " + name + " " + response.data["status"] + "!")
    })
  }

  const getTotalTimeAdmin = name => {
    if (name === "") name = user.name;

    setTime("Loading....")

    apiRequest(Axios.get, {type: "totalTime", name: name}).then((response) =>{
      console.log(response.data["totalTime"])

      setTime(Math.round(response.data["totalTime"]*100)/100 + " hours")
    })
  }

  
  const apiRequest = (http, params) => {
    let query = "?"

    if (params.type !== "signin") query += "token=" + user.token + "&"

    if (Object.keys(params).length >= 1) {
      for (const [key, value] of Object.entries(params)) {
        query += key + "=" + value + "&";
      }
      query = query.substring(0, query.length - 1);
    }

    return http(API_ENDPOINT + query)
  }

  const forgotPw = details =>{
    apiRequest(Axios.post, {type: "changepw", oldpw: details.oldpw, newpw: details.newpw}).then((response) => {
      if(response.data["result"] === "success"){
        setError("")
        setShowCPWSuccessLabel(true)
        setForgot(false)
        setChangePasswordSubmitLabel("SUBMIT")
      } else {
        setChangePasswordSubmitLabel("SUBMIT")
        setError(response.data["error"])
      }
    })

    console.log("Submitted")
  }
  
  const renderSwitch = (token) =>{
    if(forgot){
      return <ForgotPassword 
          error={error} 
          setError={setError}
          forgotPw={forgotPw}
          back={() => setForgot(false)}
          submitLabel={changePasswordSubmitLabel}
          setSubmitLabel={setChangePasswordSubmitLabel}
        />
    } else if(token !== ""){
      return <FinalForm 
          Logout={Logout} 
          requestName={requestName} 
          getTotalTime={getTotalTime} 
          signedIn={signedIn} 
          totalTime={totalTime} 
          user={user} 
          label={showCPWSuccessLabel}
          adminRequestName={adminRequestName} 
          status={status} 
          getTotalTimeAdmin={getTotalTimeAdmin} 
          changeUserPw={() => {setForgot(true); setShowCPWSuccessLabel(false);}}
        /> 
    } else {
      return <LoginForm 
          Login={Login} 
          error={error}
          signinLabel={signinLabel}
        />
    }
  }

  return (
    <div className="App">
      {renderSwitch(user.token)}      
    </div>
  );
}

export default App;