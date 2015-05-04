#!/bin/bash

pid=0
binFile=./server
pidFile=./server.pid
logFile=./server.log


printUsage() {
    echo "Usage: $0 [ start | stop | restart]"
}

startServer() {
    if [ -f $pidFile ] && kill -0 `cat $pidFile`; then
        stopServer
    fi

    if [ -f $logFile ]; then
        echo "Truncating log file..."
        cat /dev/null >$logFile
    fi

    echo "Starting server..."
    spawn-fcgi -p 9000 -n $binFile &
    pid=$!

    if [ ! -z $pid ] && kill -0 $pid; then
        echo $pid >$pidFile
        echo "Server started ($pid)."
    fi
}

stopServer() {
    echo "Stopping server..."
    if [ -f $pidFile ]; then
        if kill `cat $pidFile`; then
            if rm -f $pidFile; then
                echo "Server stopped."
            else
                echo "Server stopped, but PID file could not be removed."
            fi
        else
            echo "Unable to stop server."
        fi
    else
        echo "Unable to find server PID file."
    fi
}


case $1 in
    start)
        startServer
        ;;
    stop)
        stopServer
        ;;
    restart)
        stopServer
        startServer
        ;;
    *)
        printUsage
        ;;
esac
