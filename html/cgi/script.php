#!/usr/bin/php-cgi
<?php
// Print required HTTP header FIRST
echo "Content-type: text/html\r\n";
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
?>

<?php
// Handle POST data  
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "<p>POST data received:</p>";
    echo "<pre>" . htmlspecialchars(print_r($_POST, true)) . "</pre>";
}
?>

</body></html>
<?php
?>