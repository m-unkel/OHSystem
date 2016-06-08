<?php
if (!isset($website) ) { header('HTTP/1.1 404 Not Found'); die; }

$return = "";
$ChatOpen = 0;
$SelChatID = 0; $SelBotID = 1;

	//GAME TYPES/ALIASES (dota, lod)
	
    $sth = $db->prepare("SELECT * FROM ".OSDB_ALIASES." ORDER BY alias_id ASC");
	$result = $sth->execute();
	$GameAliases = array();
	$c = 0;
	while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {
	 $GameAliases[$c]["alias_id"] = $row["alias_id"];
	 $GameAliases[$c]["alias_name"] = $row["alias_name"];
	 
	 if ( isset($_GET["game_type"]) AND $_GET["game_type"] == $row["alias_id"] )
	 $GameAliases[$c]["selected"] = 'selected="selected"'; else $GameAliases[$c]["selected"] = '';
	 
	 if ( !isset($_GET["game_type"]) AND $row["default_alias"] == 1) {
	 $GameAliases[$c]["selected"] = 'selected="selected"';
	 $DefaultGameType = $row["alias_id"];
	 }
	 
	 $c++;
	}
	
 
if ( isset($_SESSION["restricted"])) { 
	$PageDisabled = "style='display:none;'";
	$InfoMessage =$lang["gl_no_accesss"];
	$disabled = 'disabled="disabled"';
	} else { $PageDisabled = "";  $InfoMessage = ""; $disabled = ""; }

  if ( isset($_GET["c"]) AND isset($_GET["id"]) AND is_numeric($_GET["id"]) AND isset($_GET["l"]) AND isset($_GET["hash"]) AND md5($_GET["c"]) == $_GET["hash"] ) {
  
    $chatWith = strip_tags( trim($_GET["c"]) );
	$ChatOpen = 1;
	
	if ( isset($_POST["SendSubmit"]) AND isset($_POST["commandSend"])  ) {
      
	  $sendTo = trim( htmlentities($_GET["c"]) );
	  $message = trim( strip_tags( htmlentities($_POST["commandSend"]) ) );
	  $message = substr( $message,0,400 );
	  
	  
	  $command = "!say /w $chatWith [".$_SESSION["username"]."]: $message ";
	  if ( strlen($message)>=2 ) {
	    $db->insert( OSDB_COMMANDS_QUEUE, array(
	     "botid" => 1,
		 "userid" => $_SESSION["user_id"],
	     "command" => $command
        ));
		
		$InsertID = $db->lastInsertId();
		$return = "<div>".$lang["chat_successfull"]."</div>";
		$return.= "<div><b>[".$_SESSION["username"]."]:</b> $message</div>";
	  } else {
	    $return = $lang["chat_error1"];
	  }
    }	
	
	if ( !os_is_logged() ) {
	  $dis = 'disabled';
	  $message = $lang["you_must_be"]." <a href='".OS_HOME."?login'>".$lang["logged_in"]."</a> ".$lang["to_use_feature"];
	} else { $dis = ""; $message = ""; }
	
} else {

     if ($LiveGamesLimit>=1) $LIMIT = "LIMIT $LiveGamesLimit";
	 
     $sql = " ";
	 //Limit to botid 1 - for other users - SHOW ALL for admins
	 //if (os_is_logged() AND $_SESSION["level"]>=9 ) $sql = " ";
	 //else $sql = " AND botid = 1";
	 
     $sth = $db->prepare( "SELECT * FROM ".OSDB_GAMESTATUS." WHERE gamestatus<=2 
	 AND gametime>=NOW()-INTERVAL 2 hour
	 $sql
     ORDER BY gamestatus ASC, gametime DESC $LIMIT"  );
	  
  $result = $sth->execute();
  $IDS = array();
  $c = 0;
  $GameName = "";
  $GameNameSelected = "";
     while ($row = $sth->fetch(PDO::FETCH_ASSOC)) {
	$botID = $row["botid"];
	$chatID = $row["gameid"];
	$GameStatus = $row["gamestatus"];
	$GameTime = $row["gametime"];
	$GameName = os_strip_quotes($row["gamename"]);
	
	if ( isset($_GET["gameid"]) AND is_numeric($_GET["gameid"]) AND $chatID == $_GET["gameid"] ) {
	$GameNameSelected = os_strip_quotes($row["gamename"]);
	}
	
      if ($chatID>=1 ) {
	  $IDS[$c]["chatid"] = $chatID;
	  if ($GameStatus <= 1 ) { $pr = "Lobby";$gamebutton = " LobbyButton"; }
	  if ($GameStatus == 2 ) { $pr = "Game"; $gamebutton = " GameButton"; }
	  $IDS[$c]["chatid"] = $chatID; 
	  $IDS[$c]["status"] = $pr; 
	  $IDS[$c]["button"] = $gamebutton;
	  $IDS[$c]["gn"] = $GameName;
	  $IDS[$c]["botid"] = $botID; 
	  $c++; 
	  }
    }

}
?>