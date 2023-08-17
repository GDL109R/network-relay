<?php
    // Check if correct syntax given
    if (!(isset($_POST['phpFunction']))) {
        return;
    }

    // Send message depending on variables recieved
    if ($_POST['phpFunction'] == "start") {
        send("on");
    } else {
        send("off");
    }

    // Create a socket and send UDP packet to IP:PORT with data arguement
    function send($data) {
        // Address specific to ESP-32
        $address = "192.168.1.200";
        $port = 1234;
        $sock = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
        $len = strlen($data);
        socket_sendto($sock, $data, $len, 0, $address, $port);
        socket_close($sock);

        // Send confirmation to console (debugging only)
        echo "SENT: " . $data;
    }
