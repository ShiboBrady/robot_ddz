var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var fs = require('fs');
var multer = require('multer');
var child_process = require('child_process');

var app = express();
var urlencodedParser = bodyParser.urlencoded({extended: false})
app.use(urlencodedParser);
app.use(multer({dest:'/tmp/'}).array('image'));
app.use(cookieParser());
app.use(express.static('public'));
app.set('port', process.env.PORT || 8000);

var bashPath = '/home/zhangsb/bin/test/'
var pidFile = 'pid'

var errorInfo = {
    0 : "success",
    1 : "exist pid file",
    2 : "not exist exe file",
    3 : "set core space failed",
    4 : "write pid to file failed",
    5 : "start robot program failed",
    6 : "not exist pid folder",
    7 : "not exist pid file",
    8 : "pid file is empty",
    9 : "stop robot program failed",
    10: "second params not exist",
    11: "useless second param",
    12: "param error"
}

app.get('/', function(req, res){
    res.sendFile(__dirname + "/" + "index.html");
})

app.get('/index.html', function(req, res){
    res.sendFile(__dirname + "/" + "index.html");
})

app.get('/robot_status', function(req, res){
    console.log('query robot status.');
    var workerProcess = child_process.exec('cd ' + bashPath + ' && ./s.sh ' + 'status', function (error, stdout, stderr) {
        var response;
        if (error){
            response = {
                ErrorCode: error.code,
                Msg: errorInfo[error.code]
            };
            console.log(error.stack);
            console.log(response);
        }
        else{
            console.log(stdout);
            var statueList = stdout.split('\n');
            statueList.pop();
            response = {
                count: statueList.length,
                info : statueList
            }
        }
        res.end(JSON.stringify(response));
    });
    workerProcess.on('exit', function (code) {
        console.log('child progress exit code: ' + code);
    });
})

app.get('/robot_start', function(req, res){
    var param = req.query.kind;
    console.log('start robot for ' + param);
    var workerProcess = child_process.exec('cd ' + bashPath + ' && ./s.sh ' + 'start ' + param, function (error, stdout, stderr) {
        console.log('start command execute over.');
        if (error){
            response = {
                ErrorCode: error.code,
                Msg: errorInfo[error.code]
            };
        }else{
            response = {
                ErrorCode : '0',
                Msg: 'success'
            };
        }
        console.log(response);
        res.end(JSON.stringify(response));
    });
    workerProcess.on('exit', function (code) {
        console.log('child progress exit code: ' + code);
    });
})

app.get('/robot_stop', function(req, res){
    var param = req.query.kind;
    console.log('stop robot for ' + param);
    var workerProcess = child_process.exec('cd ' + bashPath + ' && ./s.sh ' + 'stop ' + param, function (error, stdout, stderr) {
        if (error){
            response = {
                ErrorCode: error.code,
                Msg: errorInfo[error.code]
            };
        }else{
            response = {
                ErrorCode : '0',
                Msg: 'success'
            };
        }
        console.log(response);
        res.end(JSON.stringify(response));
    });
    workerProcess.on('exit', function (code) {
        console.log('child progress exit code: ' + code);
    });
})

var server = app.listen(app.get('port'), function(){
    var host = server.address().address;
    var port = server.address().port;

    console.log("http:\/\/%s:%s", host, port);
})