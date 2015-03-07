var configuration = localStorage.getItem('configValueV1.3') ? 
    localStorage.getItem('configValueV1.3') : 
'{"project":"https://www.kickstarter.com/projects/597507018/pebble-time-awesome-smartwatch-no-compromises","fontRNS":true,"fontHWT":false}';


var divRegx = function (id, dataAttr, string) {
    var idRegx = new RegExp('<div .*id="' + id + '">');
    var attrRegx = new RegExp(dataAttr + '="[0-9.]+"');
    return string.match(idRegx)[0].match(attrRegx)[0].match(/[0-9\.]+/g)[0];
};

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function getKickstarter() {
    // If it doesn't find a URL it uses the Default to prevent xhrRequest from breaking.
    var url = JSON.parse(configuration).project ? JSON.parse(configuration).project : "https://www.kickstarter.com/projects/597507018/pebble-time-awesome-smartwatch-no-compromises";
//     console.log(url);
    
    xhrRequest(url, 'GET', 
        function(responseText) {

            // Get the total pledged
            var pledged = parseInt(divRegx("pledged", "data-pledged", responseText),10);
            console.log("Pledged is " + pledged);

            // Conditions
            var backers = parseInt(divRegx("backers_count", "data-backers-count", responseText),10);     
            console.log("Backers are " + backers);
            
            var style2 = +JSON.parse(configuration).fontRNS;
            var style3 = +JSON.parse(configuration).fontHWT;
            // Assemble dictionary using our keys
            var dictionary = {
                "KEY_PLEDGED": pledged,
                "KEY_BACKERS": backers,
                "STYLE_1": 0,
                "STYLE_2": style2,
                "STYLE_3": style3
            };

            // Send to Pebble
            Pebble.sendAppMessage(dictionary,
                function(e) {
                    console.log("Kickstarter info sent to Pebble successfully!");
                },
                function(e) {
                    console.log("Error sending kickstarter info to Pebble!");
                }
            );
        }      
    );
}



// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log("PebbleKit JS ready!");
        getKickstarter();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function(e) {
        console.log("AppMessage received!");
        getKickstarter();
    }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
    console.log('Input Config: ' + '"' + configuration + '"');
    Pebble.openURL('http://pebble.tomprice.ca/pebbleConfig?' + encodeURIComponent(configuration));
});

Pebble.addEventListener('webviewclosed', function(e) {
    if (e.response == "") {
        console.log("Empty response");
    } else {
        console.log('Outpt Config: ' + '"' + e.response + '"');
        configuration = decodeURIComponent(e.response);
        localStorage.setItem('configValueV1.3', configuration);
        getKickstarter();
    }
});
