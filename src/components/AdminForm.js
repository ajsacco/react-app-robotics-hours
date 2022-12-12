import React, {useState} from 'react'
import Axios from 'axios'

function AdminForm( {Logout, adminRequestName, status, totalTime, getTotalTimeAdmin}) {

const [details, setDetails] = useState({name: ""})

    // const adminRequestName = name => {
    //     // setStatus("Loading...")
    //     // Axios.get("https://script.google.com/macros/s/AKfycbzrZrUGdpTtUfUF2XYvUuSMQk6-LUB839QOnkUfuZcv07LbudNkJjSq6Vg22luMY7SL6w/exec?token=52fa80662e64c128f8389c9ea6c73d4c&type=logname&name="+name)
    //     // setStatus("You've Signed in!")

    //     console.log("YOUVE SIGNED IN    ")
    // }


    return (
        <div className = "admin"> 
            <h2>Admin Panel</h2>
            <h2></h2>
            <h2>{totalTime}</h2>
            <h2></h2>
            <h2>{status}</h2>
            <h2></h2>
            <label htmlFor='name'> Name: </label>
            <input type="text" name = "Name" id = "name"/>
            <h2></h2>
            <button onClick={Logout}> Logout</button>
            <h2></h2>
            <button onClick={() => adminRequestName(document.getElementById("name").value)}> Log In/Out</button>
            <h2></h2>
            <button onClick={() => getTotalTimeAdmin(document.getElementById("name").value)}> Total Time</button>
        </div>
    )


}

export default AdminForm