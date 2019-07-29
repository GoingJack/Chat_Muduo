#pragma once

/*
公共头文件，统一服务器和客户端的常量定义
*/
const int MSG_LOGIN = 0; // login msg
const int MSG_LOGIN_ACK = 1; // login ack msg
const int MSG_REG = 2; // register msg
const int MSG_REG_ACK = 3; // register ack msg

const int MSG_ADD_FRIEND = 4; // add friend msg
const int MSG_ADD_FRIEND_ACK = 5; // add friend ack msg

const int MSG_ADD_GROUP = 6; // add group msg
const int MSG_ADD_GROUP_ACK = 7; // add group ack msg

const int MSG_ONE_CHAT = 8; // one to one chat msg
const int MSG_ONE_CHAT_ACK = 9; // one to one chat ack msg

const int MSG_LOGINOUT = 10; // logout msg

const int MSG_REQUEST_FRIENDLIST_ACK = 11;//request friend list ack msg
const int MSG_REQUEST_FRIENDLIST = 12;//request friend list msg

const int MSG_LOGOUT = 13;
const int MSG_LOGOUT_ACK = 14;

const int MSG_REQUEST_ONLINE_FRIEND = 14;
const int MSG_REQUEST_ONLINE_FRIEND_ACK = 15;

const int MSG_ADD_FRIEND_EXIST = 16; // in order to verify friend id is or is not exist
const int MSG_ADD_FRIEND_EXIST_ACK = 17; // in order to verify friend id is or is not exist

const int MSG_SHOW_ALL_REQUEST = 18;//show all request 
const int MSG_SHOW_ALL_REQUEST_ACK = 19;//ack msg for show all request

const int MSG_ACK_ADD_FRIEND = 20;
const int MSG_ACK_ADD_FRIEND_ACK = 21;

const int ACK_SUCCESS = 100; // msg process ok
const int ACK_ERROR = 101; // msg process err