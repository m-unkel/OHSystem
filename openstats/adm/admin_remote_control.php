<?php
if (!isset($website) ) { header('HTTP/1.1 404 Not Found'); die; }
$errors = "";

?>
<div align="center">
<h2>Remote Bot Control</h2>
</div>
<?php
if (is_logged() AND isset($_SESSION["level"] ) AND $_SESSION["level"]<=9 ) {
?>
<div align="center">
<h2>Only root administrators can run this option</h2>
</div>
<?php
} else {

   if ( isset($_GET["clear_messages"]) ) {
    $sth = $db->prepare( "TRUNCATE TABLE ".OSDB_COMMANDS_QUEUE.""  );
    $result = $sth->execute();
	?>
    <div align="center">
	<h2>All messages are deleted successfully.</h2>
	<a href="<?=OS_HOME?>adm/?remote">Refresh page</a> to continue.
	</div>
	<?php
	OS_AddLog($_SESSION["username"], "[os_rcon] Removed all remote commands ");
   }

 if ( isset($_POST["rc"]) AND isset($_POST["botid"]) AND is_numeric($_POST["botid"]) AND isset($_POST["command"]) ) {
    $botid = safeEscape( (int) $_POST["botid"] );
	$command = strip_tags( (trim( $_POST["command"] ) ) );
	$db->insert( OSDB_COMMANDS_QUEUE, array(
	"botid" => $botid,
	"userid" => $_SESSION["user_id"],
	"command" => $command
     ));
	 
	$InsertID = $db->lastInsertId();
	?>
	<div align="center">
	<?php if ($InsertID>=1) { 
	OS_AddLog($_SESSION["username"], "[os_rcon] Sent Remote command ( #$InsertID )");
	?>
	  Message #<?=$InsertID?> has been successfully sent.
	  <div style="font-size:11px;"><?=$command?></div>
	<?php } else { ?>
	Error. Something is wrong. Make sure you <a href="http://www.codelain.com/forum/index.php?topic=17803.0;" target="_blank">install the patch</a>.
	<?php } ?>
	</div>
	<?php
 }

$value = "";
if ( isset($_GET["m"]) ) $value = "!say /w ".strip_tags($_GET["m"])." ";
 ?>
 <div align="center">
   <form action="" method="post">
     <table>
	    <tr>
		  <td>BotID:</td>
		  <td><input type="text" value="1" name="botid" size="1" /></td>
		</tr>
	    <tr>
		  <td>Command:</td>
		  <td><input id="botCommand" type="text" value="<?=$value?>" name="command" size="80" /></td>
		</tr>
	    <tr>
		  <td>Help commands</td>
		  <td>
		  <a href="javascript:;" onclick="CommandHelp('!say ')">[say]</a>
		  <a href="javascript:;" onclick="CommandHelp('!say /w ')">[whisper]</a>
		  <!--
		  <a href="javascript:;" onclick="CommandHelp('!load ')">[load]</a>
		  <a href="javascript:;" onclick="CommandHelp('!map ')">[map]</a>
		  -->
		  <a href="javascript:;" onclick="CommandHelp('!pub ')">[pub]</a>
		  <a href="javascript:;" onclick="CommandHelp('!priv ')">[priv]</a>
		  <!--
		  <a href="javascript:;" onclick="CommandHelp('!unban ')">[unban]</a>
		  -->
		  <a href="javascript:;" onclick="CommandHelp('!rcon mute <?=$_SESSION["username"]?> ')">[mute]</a>
		  <a href="javascript:;" onclick="CommandHelp('!rcon unmute <?=$_SESSION["username"]?> ')">[unmute]</a>
		  <a href="javascript:;" onclick="CommandHelp('!rcon kick <?=$_SESSION["username"]?> ')">[kick]</a>
		  
		  <a href="javascript:;" onclick="CommandHelp('!rcon saylobby <?=$_SESSION["username"]?> ')">[ChatLobby]</a>
		  <a href="javascript:;" onclick="CommandHelp('!rcon lobbyteam <?=$_SESSION["username"]?> 1/0')">[LobbyTeam]</a>
		  <a href="javascript:;" onclick="CommandHelp('!rcon saygame <?=$_SESSION["username"]?> gnumber ')">[ChatGame]</a>
		  <a href="javascript:;" onclick="CommandHelp('!rcon gameteam <?=$_SESSION["username"]?> 1/0')">[ChatTeamGame]</a>
		  </td>
		</tr>
	    <tr>
		  <td></td>
		  <td>
		  <input type="submit" value="Send" name="rc" class="menuButtons" />
		  <input type="button" value="Clear all messages" class="menuButtons" onclick="if (confirm('This will truncate table and delete all messages. Are you sure?') ) { location.href='<?=OS_HOME?>adm/?remote&clear_messages' }" />
		  </td>
		</tr>
	 </table>
   </form>
 </div>
 <?php
}
?> 

<div align="right" style="margin-top:32px; font-size:12px; margin-right:24px;">
*Require <a href="http://www.codelain.com/forum/index.php?topic=17803.0;" target="_blank">Commands through MySQL</a> patch (by uakf.b) 
</div>

<div style="margin-top: 230px;">&nbsp;</div>