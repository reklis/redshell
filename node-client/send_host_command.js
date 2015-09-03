#!/usr/bin/env node

var zmq = require('zmq')
  , sock = zmq.socket('req')
  , fs = require('fs')
  , settings = require('./settings')
  , args, cmd, cmd_params

// connect to host
sock.connect(settings.socket)

// exit on host ACK
sock.on('message', function (msg) {
  process.exit()
})

// first param is the node interpreter
// second param is script name
args = process.argv
args.shift()
args.shift()

// strip off the command to send
cmd = args[0]
args.shift()

// the rest of the arguments are command parameters
args.forEach(function (arg) {
  var local_path, remote_path

  try {
    // assume it's a file
    local_path = fs.realpathSync(arg)

    // replace all the mount points with host paths
    remote_path = local_path
    settings.mountpoints.forEach(function (mountpoint) {
      remote_path = remote_path
        .replace(mountpoint.vm, mountpoint.host)
    })

    // fix the path separators
    remote_path = remote_path.replace(/\//g, '\\')

    // console.log('local: ' + local_path)
    // console.log('remote: ' + remote_path)

    cmd_params += ' ' + remote_path
  } catch (ex) {
    // if it's not a file, just treat it as an arg
    cmd_params += ' ' + arg
  }

})

sock.send(cmd + '\v' + cmd_params)
