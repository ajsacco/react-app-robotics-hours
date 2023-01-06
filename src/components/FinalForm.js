import React, {useState} from 'react'
import AdminForm from './AdminForm'
import ForgotPassword from './ForgotPassword'
import MuggleForm from './MuggleForm'
import UserForm from './UserForm'

function FinalForm( {Logout, requestName, getTotalTime, signedIn, totalTime, user, adminRequestName, status, getTotalTimeAdmin, changeUserPw, label}) {

    if(user.level === "user"){
        return(          
            <UserForm Logout = {Logout} requestName = {requestName} getTotalTime = {getTotalTime} signedIn = {signedIn} totalTime = {totalTime} user = {user} adminRequestName = {adminRequestName} changeUserPw={changeUserPw} label={label}/>
        )
    }

    if(user.level === "admin"){
        return(
            <AdminForm Logout={Logout} adminRequestName={adminRequestName} status = {status} totalTime = {totalTime} getTotalTimeAdmin={getTotalTimeAdmin} changeUserPw={changeUserPw} label={label}/>
        )
    }

    if(user.level === "limited"){
        return(
            <MuggleForm Logout={Logout} totalTime={totalTime} getTotalTime={getTotalTime} user = {user} changeUserPw={changeUserPw} label={label}/>
        )
    }
}

export default FinalForm