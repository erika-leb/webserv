#!/usr/bin/php-cgi
<?php
// Print required HTTP header FIRST
echo "Content-Type: text/html\r\n";
echo "\r\n";

?>
<html><body>
<h1>Hello from PHP CGI!</h1>

<?php
// Handle GET data
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    echo "<p>Query String: " . htmlspecialchars($_SERVER['QUERY_STRING']) . "</p>";
    echo "<p>GET params:<br>";
    foreach ($_GET as $key => $value) {
        echo htmlspecialchars($key) . " = " . htmlspecialchars($value) . "<br>";
    }
}
else {
	echo "blooooo";
}
?>

<?php
// Handle POST data  
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "<p>POST data received:</p>";
	$input = file_get_contents('php://stdin');  // POST body here!
	echo "POST body length: " . strlen($input);
}
else {
	echo "blaaaaaaaa";
}
?>

</body></html>
<?php
?>