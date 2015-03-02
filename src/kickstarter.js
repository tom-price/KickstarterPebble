var configuration = localStorage.getItem('configValue') ? 
    localStorage.getItem('configValue') : 
'%7B%22project%22%3A%22https%3A%2F%2Fwww.kickstarter.com%2Fprojects%2F597507018%2Fpebble-time-awesome-smartwatch-no-compromises%22%2C%22fontRNS%22%3Atrue%2C%22fontOCR%22%3Afalse%7D';


var divRegx = function (id, dataAttr, string) {
    var idRegx = new RegExp('<div .*id="' + id + '">');
    var attrRegx = new RegExp(dataAttr + '="[0-9\.]+"');
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

    var url = "https://www.kickstarter.com/projects/597507018/pebble-time-awesome-smartwatch-no-compromises";
    xhrRequest(url, 'GET', 
        function(responseText) {

            // Get the total pledged
            var pledged = parseInt(divRegx("pledged", "data-pledged", responseText),10);
            console.log("Pledged is " + pledged);

            // Conditions
            var backers = parseInt(divRegx("backers_count", "data-backers-count", responseText),10);     
            console.log("Backers are " + backers);

            // Assemble dictionary using our keys
            var dictionary = {
                "KEY_PLEDGED": pledged,
                "KEY_BACKERS": backers
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
  // Show config page
    console.log('  coded configuration: ', configuration);
    console.log('Decoded configuration: ', decodeURIComponent(configuration));
    Pebble.openURL('https://dl.dropboxusercontent.com/u/29210594/config.html?' + configuration);
});

Pebble.addEventListener('webviewclosed', function(e) {
    var options = decodeURIComponent(e.response);
    configuration = encodeURIComponent(options);
    localStorage.setItem('configValue', configuration);
    
    console.log('Getfrom Local Storage', localStorage.setItem('configValue'));
    
    getKickstarter();
});
