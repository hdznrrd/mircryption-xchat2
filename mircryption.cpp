//---------------------------------------------------------------------------
// Based on www.xchat.org provided plugin header - works ONLY with new plugin
//  system for xchat versions >= 2.x
//
// http://mircryption.sourceforge.net
// By Dark Raichu and Xro, 1/03 ...
//---------------------------------------------------------------------------
// Contributors:
//	Raichu
//	Xro
//	gjehle		Gregor Jehle <gjehle@gmail.com>
//---------------------------------------------------------------------------
//  4/27/03 fixed lines with / not being encrypted
//  4/28/03 added \017 characters to format, which allows nicks to be clicked.
//  5/09/03 changed default keyfile location to ~/.xchat/MircryptionKeys.txt
//  5/13/03 added windows compatibility (tested with xygwin and xchat2 for win32)
//  9/08/03 added support for ` prefix toggle
// 12/27/03 gjehle: added color stripping

// 03/21/04 help message was saying to call '/setkey #chan key' instead of '/setkey key'
// 03/30/04 changed default outgoing tag to +OK for better default compatibility with other scripts
// 05/18/04 added meow support
// 12/26/04 fixing for xchat umlaout bug (utf8 stuff)
// 01/08/04 added new cbc blowfish modes

// 03/28/07 gjehle: Event "Notice Send" got replaced by "Your Notice", updated event hook & handler
// 03/28/07 gjehle: Added EACTION_RECV_FORMAT and EACTION_SEND_FORMAT to replace the previous one-fits-all
// 03/29/07 gjehle: We now obey XChat's foreground color settings for nicknames displayed when in crypto mode
// 03/29/07 gjehle: raised version to 0.2.0

// 04/06/07 gjehle: Now decrypting text in RAW messages to prevent accidential nicknighlighting (thanks bt!)
// 04/06/07 gjehle: Rewrote quite some of the inbound message parser to use raw and /recv-reissue w/ textevents
// 04/06/07 gjehle: nickhighlighting now works if text was encrypted, too
// 04/06/07 gjehle: Added key-migration to support nickchanges while having an encrypted converstation
// 04/06/07 gjehle: moved all xchat_printf template-formats to a global stringarray that can be accessed using enum eTextEvents
// 04/06/07 gjehle: began commenting of code in doxygen syntax
// 04/06/07 gjehle: raised version to 0.3.0

// 04/09/07 gjehle: added a delayed popup to ask for the masterkey if not already entered
// 04/09/07 gjehle: raised version to 0.3.1

// 04/16/07 gjehle: added support for idmsg servers (like freenode) who prepend messages with a + or - (thanks to Waky for reporting this bug)
// 04/16/07 gjehle: raised version to 0.3.2

// 04/16/07 gjehle: previous change messed up ACTION handling, fixed it and cleaned up the code
// 04/16/07 gjehle: changed masterkey input box delay to 1.5 seconds
// 04/16/07 gjehle: raised version to 0.3.3
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// TODO list
// id	who		what
// 1	gjehle		hook /msg so we can encrypt that stuff
// 2	gjehle		convert topic related stuff to RAW
// 3	gjehle		add DH1080 keyexchange support
// 4	gjehle		add support for runtime changeable themes (like xchat's own text events)
// 5	gjehle		rewrite rest of code to eliminate redundancy
// 6	gjehle		make it that key migration notices are displayed in the currently active window
// 7	gjehle		add support for intel-mac to makefile (bug reported by _d4vid)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// xchat plugin version
#define XMC_VERSIONSTRING "0.3.3"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// cygwin may not have WIN32 defined (which we use AND more imp xchat.h uses)
#ifndef WIN32
 #ifdef _MSC_VER
 #define WIN32
 #endif
 #ifdef __CYGWIN__
 #define WIN32
 #endif
 #ifdef __MINGW32__
 #define WIN32
 #endif
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// default key file name and location
#ifdef WIN32
 #define DEFAULTKEYFILE "MircryptionKeys.txt"
#else
 // note if no / at front then this is relative filename off of user's home dir
 // remember that the . at start of name makes the file visible only w/ ls -a
 #define DEFAULTKEYFILE ".MircryptionKeys.txt"
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// xchat function and enum definitions
#include "xchat-plugin.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Application includes
#include "mirc_codes.h"
#include "mircryption.h"
#include <stdlib.h>
//---------------------------------------------------------------------------



#define TFU	"\037"
#define TFR	"\026"
#define TFB	"\002"
#define TFO	"\017"
#define TFCOL	"\003"
#define CWHITE  TFCOL "00"
#define CBLACK	TFCOL "01"
#define CBLUE	TFCOL "02"
#define CGREEN	TFCOL "03"
#define CRED	TFCOL "04"
#define CBROWN	TFCOL "05"
#define CPURPLE	TFCOL "06"
#define CORANGE	TFCOL "07"
#define CYELLOW	TFCOL "08"
#define CLGREEN	TFCOL "09"
#define CBLUE1	TFCOL "10"
#define CBLUE2	TFCOL "11"
#define CBLUE3	TFCOL "12"
#define CPINK	TFCOL "13"
#define CGRAY1	TFCOL "14"
#define CGRAY2	TFCOL "15"

enum eTextEvents {
/*0*/	 eTeChannelMessage = 0
/*1*/	,eTeChannelMsgHilight
/*2*/	,eTeYourMessage
/*3*/	,eTeChannelAction
/*4*/	,eTeChannelActionHilight
/*5*/	,eTeYourAction
/*6*/	,eTeNotice
/*7*/	,eTeNoticeSend
/*8*/	,eTeChannelNotice
/*9*/	,eTeMessageSend
/*a*/	,eTePrivateMessage
/*b*/	,eTePrivateMessageToDialog
/*c*///	,eTeTopic
/*d*///	,eTeTopicCreation
/*e*///	,eTeTopicChange
};

//---------------------------------------------------------------------------
// Uncomment one of these pairs to choose how encrypted messages are formatted
// Default mircryption method, use [] around encrypted text
// gjehle 070406: sorry Xro, i've been to lazy to adapt this array for your style ;-) but please, go ahead
static const char* TEXT_EVENT_FORMAT[] = {
	CBLUE	 "[" TFO 	 "%s" TFO CBLUE	  "]" 		TFO "\t%s\n"	//0 eTeChannelMessage 		EMESSAGE_RECV_FORMAT
,	CBLUE	 "[" CYELLOW TFB "%s" TFO CBLUE	  "]" 		TFO "\t%s\n"	//1 eTeChannelMsgHilight
,	CPURPLE	 "[" TFO 	 "%s" TFO CPURPLE "]" 		TFO "\t%s\n"	//2 eTeYourMessage 		EMSG_SEND_FORMAT

,	CBLUE	 "[" CPINK "*" CBLUE  	"]\t"		 	TFO "%s %s\n"	//3 eTeChannelAction 		EACTION_RECV_FORMAT
,	CBLUE	 "[" CPINK "*" CBLUE  	"]\t" TFB CYELLOW "%s "	TFO    "%s\n"	//4 eTeChannelActionHilight
,	CPURPLE	 "[" CPINK "*" CPURPLE	"]\t"		 	TFO "%s %s\n"	//5 eTeYourAction 		EACTION_SEND_FORMAT

,	CBLUE	"-[" CPINK "%s" TFO CBLUE    "]-"		TFO "\t%s\n"	//6 eTeNotice			ENOTICE_RECV_FORMAT
,	CPURPLE	"-[" TFO   "%s" TFO CPURPLE  "]-"		TFO "\t%s\n"	//7 eTeNoticeSend		ENOTICE_SEND_FORMAT
,	CBLUE	"-[" CPINK "%s/%s" TFO CBLUE "]-"		TFO "\t%s\n"	//8 eTeChannelNotice		ENOTICE_RECV_CHANNEL_FORMAT

,	CPURPLE	 "]" TFO "%s" TFO CPURPLE  "[" 			TFO "\t%s\n"	//9 eTeMessageSend
,	CBLUE	 "]" TFO "%s" TFO CBLUE	   "[" 			TFO "\t%s\n"	//a eTePrivateMessage

,	CBLUE	 "[" TFO "%s" TFO CBLUE	   "]" 			TFO "\t%s\n"	//b eTePrivateMessageToDialog 	EMESSAGE_RECV_FORMAT
//,							    "Topic: %s : %s"	//c eTeTopic
//,							    "Topic: %s : %s"	//d eTeTopicCreation
//,							    "Topic: %s : %s"	//e eTeTopicChange
};

// Xro prefers a prefix of mc before <>
//#define EMESSAGEFORMAT "mc \00303>\00300%s\00303<\00300\t%s"
//#define EACTIONFORMAT "mc \00313*\00300\t%s %s"
// house uses the following:
//#define EMESSAGE_SEND_FORMAT "\00306[\017%s\00306]\017\t%s\n"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Here you can choose what outgoing tag to use; the defaul is mcps
//  but you can set it to +OK if you want to be compatible with others apps.
//  just comment out the one you *dont* want to use with a //
//#define OUTGOINGETAG "mcps"
#define OUTGOINGETAG "+OK"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// If you want to force your xchat plugin to ignore meow broadcasts, you
//  can set this to true.  Meows are just fun ways of saying hello to fello
//  mircryption users.
#define IGNOREMEOWS false
//---------------------------------------------------------------------------


/// \brief this global is used during re-issue of commands using the /recv command
static bool GLOBAL__TEXT_WAS_CRYPTED = false;

/// \brief remember if we already set the masterkey once, to prevent the input box to pop up
static bool GLOBAL__MASTERKEY_SET = false;

/// \brief the delay it takes to pop up the masterkey input box (to let automated scripts set the key first)
#define MASTERKEY_INPUT_DELAYMS 1500

//---------------------------------------------------------------------------
/// \brief global plugin handle
static xchat_plugin *ph;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// \brief pointer to global mircryptor which does the bulk of the work
MircryptionClass_xchat *mircryptor=NULL;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Forward declarations
static int setdefault_keyfilelocation();
//
static int mc_grand_unified_rawhandler(char *word[], char *word_eol[], void *userdata);
bool mc_decrypt_text(char *text_decrypted, const char *text_crypted, const char* key);
static bool mc_get_nick_from_mask(char *nick, const char *mask);

static int mc_grand_unified_eventhandler(char *word[], void *userdata);


static int mc_event_topic(char *word[], void *userdata);
static int mc_event_topicchange(char *word[], void *userdata);

static int mc_event_yourmessage(char *word[], void *userdata);
static int mc_event_youraction(char *word[], void *userdata);
static int mc_event_noticesend(char *word[], void *userdata);
static int mc_event_changenick(char *word[], void *userdata);
//
static int mc_help(char *word[], char *word_eol[], void *userdata);
static int mc_setkey(char *word[], char *word_eol[], void *userdata);
static int mc_delkey(char *word[], char *word_eol[], void *userdata);
static int mc_disablekey(char *word[], char *word_eol[], void *userdata);
static int mc_enablekey(char *word[], char *word_eol[], void *userdata);
static int mc_displaykey(char *word[], char *word_eol[], void *userdata);
static int mc_listkeys(char *word[], char *word_eol[], void *userdata);
static int mc_keypassphrase(char *word[], char *word_eol[], void *userdata);
static int mc_setkeyfile(char *word[], char *word_eol[], void *userdata);
static int mc_etopic(char *word[], char *word_eol[], void *userdata);
static int mc_alltext(char *word[], char *word_eol[], void *userdata);
static int mc_action(char *word[], char *word_eol[], void *userdata);
static int mc_notice(char *word[], char *word_eol[], void *userdata);

static int mc_timer_request_masterkey(void *userdata);

static void mylowercasify(char *str);

void strip_mirc_colors(xchat_plugin*,char*);

// meow handling
bool HandleMeow(char *channelname,char *nick,char *text);

// xchat umlaout issue (utf8 stuff)
void Utf8DirtyFix(char *intext,char *outtext);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// new extra safe buffer overflow checks
// thanks to www.rainbowcrack-online.com and ircfuz for finding this dangeroud possibility
//  more serious buffer overflow checks are also now in the dll as well
void mcnicksafe_strcpy(char *dest,const char*src) {mcsafe_strcpy(dest,src,MAXCHANNELNAMESIZE);};
void mclinesafe_strcpy(char *dest,const char*src) {mcsafe_strcpy(dest,src,MAXLINELEN);};
void mckeysafe_strcpy(char *dest,const char*src) {mcsafe_strcpy(dest,src,MAXSAFEKEYSIZE);};
void mcsafe_strcpy(char *dest,const char *src, int maxlen) {if (strlen(src)<maxlen-1) strcpy(dest,src); else {strncpy(dest,src,maxlen-1);dest[maxlen-1]='\0';};};
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// do we need a dummy main for some compilers
// ATTN: new 07/10/03
// int main(int argc,void **argv) {return 0;}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
extern "C" int xchat_plugin_init(xchat_plugin *plugin_handle, char **plugin_name, char **plugin_desc, char **plugin_version, char *arg)
{
	// plugin is being loaded

	// save this in order to call xchat_* functions
	ph = plugin_handle;

	// xchat plugin info
	*plugin_name = "mircryption";
	*plugin_desc = "Mircryption - cryptographic addon for mirc/xchat (http://mircryption.sourceforge.net)";
	*plugin_version = XMC_VERSIONSTRING;

	// hook the commands we provide
	xchat_hook_command(ph, "mircryption", PRI_HIGHEST, mc_help, "Usage: MIRCRYPTION, shows help for mircryption", 0);
	xchat_hook_command(ph, "setkey", PRI_HIGHEST, mc_setkey, "Usage: SETKEY keyphrase..,  enables encryption/decryption on current channel, using key specified; can be used to add or modify keys.", 0);
	xchat_hook_command(ph, "delkey", PRI_HIGHEST, mc_delkey, "Usage: DELKEY, removes encryption key from current channel.", 0);
	xchat_hook_command(ph, "disablekey", PRI_HIGHEST, mc_disablekey, "Usage: DISABLEKEY, temporarily disables encryption for current channel", 0);
	xchat_hook_command(ph, "enablekey", PRI_HIGHEST, mc_enablekey, "Usage: ENABLEKEY, re-enables encryption for current channel", 0);
	xchat_hook_command(ph, "displaykey", PRI_HIGHEST, mc_displaykey, "Usage: DISPLAYKEY, shows you (and only you) the key for the current channel", 0);
	xchat_hook_command(ph, "listkeys", PRI_HIGHEST, mc_listkeys, "Usage: LISTKEYS, lists all channel encryption keys currently stored", 0);
	xchat_hook_command(ph, "keypassphrase", PRI_HIGHEST, mc_keypassphrase, "", 0); // legacy
	xchat_hook_command(ph, "masterkey", PRI_HIGHEST, mc_keypassphrase, "Usage: MASTERKEY phrase.., set or change current master keyfile passphrase to 'phrase'", 0);
	xchat_hook_command(ph, "setkeyfile", PRI_HIGHEST, mc_setkeyfile, "Usage: SETKEYFILE filename, set the name of the file to be used for storing/retrieving keys", 0);
	xchat_hook_command(ph, "etopic", PRI_HIGHEST, mc_etopic, "Usage: ETOPIC text.., encrypt the topic for the current channel to text", 0);
	xchat_hook_command(ph, "me", PRI_HIGHEST, mc_action, "Usage: ME <action>", 0);
	xchat_hook_command(ph, "notice", PRI_HIGHEST, mc_notice, "Usage: NOTICE <nick/channel> <message>, sends a notice. Notices are a type of message that should be auto reacted to", 0);
	xchat_hook_command(ph, "", PRI_HIGHEST, mc_alltext, "trap all input for encryption", 0);

	// hook the events we want to respond to
	// gjehle 070405: we actually have to hook the PRIVMSG and NOTICE replies to properly handle incoming text
	//			otherwise text might be highlighted and thus not parsed first (in case nick matches inside the base64 string)
	// gjehle 070405: magic magic, one-code-fits-all PRIVMSG handler
	xchat_hook_server(ph, "PRIVMSG", PRI_HIGHEST, mc_grand_unified_rawhandler, 0);
	xchat_hook_server(ph, "NOTICE",  PRI_HIGHEST, mc_grand_unified_rawhandler, 0);
	//xchat_hook_server(ph, "NICK",  PRI_HIGHEST, mc_grand_unified_rawhandler, 0);
	//xchat_hook_server(ph, "TOPIC",  PRI_NORM, mc_event_rawnotice, 0);
	//xchat_hook_server(ph, "332",  PRI_NORM, mc_event_rawnotice, 0);

	xchat_hook_print(ph, "Channel Message"		, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Channel Msg Hilight"	, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Channel Notice"		, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Channel Action"		, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Channel Action Hilight"	, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Notice"			, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Private Message"		, PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	xchat_hook_print(ph, "Private Message to Dialog", PRI_HIGHEST, mc_grand_unified_eventhandler, 0);
	//xchat_hook_print(ph, "", PRI_HIGHEST, mc_grand_unified_eventhandler, 0);

	// gjehle 070406 TODO: convert topic code to RAW
	xchat_hook_print(ph, "Topic", PRI_HIGHEST, mc_event_topic, 0);
	xchat_hook_print(ph, "Topic Change", PRI_HIGHEST, mc_event_topicchange, 0);
	xchat_hook_print(ph, "Your Message", PRI_HIGHEST, mc_event_yourmessage, 0);
	xchat_hook_print(ph, "Your Action", PRI_HIGHEST, mc_event_youraction, 0);
	xchat_hook_print(ph, "Notice Send", PRI_HIGHEST, mc_event_noticesend, 0);
	xchat_hook_print(ph, "Change Nick", PRI_HIGHEST, mc_event_changenick,0 );

	xchat_hook_timer(ph, 4000, mc_timer_request_masterkey, 0);

	// create our mircryptor class which does the bulk of the work
	mircryptor=new MircryptionClass_xchat;

	// set keyfile to ~/.MircryptionKeys.txt
	setdefault_keyfilelocation();

	// if keyfile is not yet unlocked, then unlock them
	mircryptor->load_keys();

	// inform user that the addon is loaded
	xchat_printf(ph, "Mircryption ver %s loaded - encryption currently *disabled*\n",XMC_VERSIONSTRING);
	xchat_printf(ph, " type /masterkey PASSPHRASE to activate, or /mircryption for help.\n");
	
	// return 1 for success
	return 1;
}


extern "C" int xchat_plugin_deinit (xchat_plugin *plugin_handle)
{
	// plugin needs to shut down

	// free mircryptor object
	if (mircryptor!=NULL)
		delete mircryptor;
	mircryptor=NULL;

	// return 1 for success
	return 1;
}
//---------------------------------------------------------------------------

/// \brief request the masterkey using a little popup input box but only if it hasn't been set already
/// \author Gregor Jehle <gjehle@gmail.com>
/// \param word see xchat documentation
/// \param word_eol see xchat documentation
/// \param userdata see xchat documentation
/// \return 1 to keep timer going, 0 to stop
static int mc_timer_request_masterkey(void *userdata)
{
	if(!GLOBAL__MASTERKEY_SET)
		xchat_command(ph,"getstr \" \" \"masterkey\" \"Please enter your Mircryption masterkey:\"");
	return 0; // only ask once
}


//---------------------------------------------------------------------------
static int setdefault_keyfilelocation()
{
	// set default key file location
	char returndata[MAXRETURNSTRINGLEN];
	char keyfile[1000];

	// form default keyfile name
	strcpy(keyfile,DEFAULTKEYFILE);

    #ifndef WIN32
	if (keyfile[0]!='/')
		{
		// we need to grab users home explicitly, using ~/ didnt work
		strcpy(keyfile,getenv("HOME"));
		if (strlen(keyfile)>0)
			strcat(keyfile,"/");
		strcat(keyfile,DEFAULTKEYFILE);
		}
	#endif
	
	mircryptor->mc_setkeyfilename(keyfile,returndata);
}
//---------------------------------------------------------------------------



/// \brief the grand unified RAW parser, this is the place that parses pretty much all incoming raw data that we hooked
/// \author Gregor Jehle <gjehle@gmail.com>
/// \param word see xchat documentation
/// \param word_eol see xchat documentation
/// \param userdata see xchat documentation
/// \return EAT_ALL or EAT_NONE (see xchat documentation)
static int mc_grand_unified_rawhandler(char *word[], char *word_eol[], void *userdata)
{
	/// \brief a little array access helper. this is valid for all commands with targets (eg. doesn't work for NICK or RAW332)
	enum eRawParse { RP_ID = 0
			,RP_SOURCE
			,RP_CMD
			,RP_TARGET
			,RP_TEXT
			};

	/// \brief what's the visibility, was the target a channel or was it us directly?
	enum eTarget {   TR_UNDEF = 0
			,TR_CHANNEL
			,TR_PRIVATE 
			 };

	// some buffers we might need
	char nick_buf[MAXCHANNELNAMESIZE];
	char key_buf[MAXCHANNELNAMESIZE];
	char text_crypted_buf[MAXLINELEN];
	char text_decrypted_buf[MAXRETURNSTRINGLEN];
	// a little view into the crypted buffer, that way it's easier to decrypt ACTIONs
	char *text_crypted_v	=NULL;

	// some stuff we need to handle idmsg formatting (eg. on freenode)
	bool msg_type_action=false;
	bool serv_idmsg=false;
	char serv_idmsg_type[]="\0"; // this is a char* for a reason ;-) trickery further down

	// this call is caused by re-issuing the decrypted text using /recv
	// we'll return EAT_NONE to then later catch the text event again
	if(GLOBAL__TEXT_WAS_CRYPTED)
		return EAT_NONE;

	// !!! if we're here, it's the first time the handler is called for this line of text

	enum eTarget raw_target = TR_UNDEF;


	// extract some data we might need

	// check who was the target
	raw_target = (word[RP_TARGET][0] == '#' || word[RP_CMD][0] == '&') ? TR_CHANNEL : TR_PRIVATE;

	if(!mc_get_nick_from_mask(nick_buf,word[RP_SOURCE]+1)) {
		xchat_printf(ph,"MCPS: ERROR: failed to extract nickname from hostmask \"%s\"",word[RP_SOURCE]+1);
		goto exit_eat_none;
	}

	// now that we know who's targeted ;-)
	// we can extract the data that's used to look-up the crypto-key
	mcnicksafe_strcpy(key_buf,(raw_target == TR_CHANNEL) ? word[RP_TARGET] : nick_buf);

	// make sure our key is in lower case
	mylowercasify(key_buf);

	mclinesafe_strcpy(text_crypted_buf,word_eol[RP_TEXT]+1);
	text_crypted_v = text_crypted_buf;


	// some straight forward parsing for the different kinds of incoming messages
	// a state machine could be written, but given that we usually deal with low traffic it's no use
	// KISS ;-)
	// imho this pretty much optimizes speed and maintainability
	msg_type_action=false;
	serv_idmsg=false;

msg_iscrypt_match:
	if(		strncmp(text_crypted_v,"+OK ",4) == 0 ||
			strncmp(text_crypted_v,"mcps ",5) == 0 ) {
		// normal crypted message, classic mircryption style
	}
	else if(	strncmp(text_crypted_v,"\001ACTION +OK ",12) == 0 ||
			strncmp(text_crypted_v,"\001ACTION mcps ",13) == 0 ) {
		// normal action, classic, classic mircryption style
		msg_type_action=true;
		text_crypted_v+=8;
		text_crypted_v[strlen(text_crypted_v)-1]='\0';
	}
	else if( !serv_idmsg && strlen(text_crypted_v) > 1 && ( *text_crypted_v == '+' || *text_crypted_v == '-' ) ){
		// server has idmsg support (eg. freenode)
		// remember the type of ID sign and skip it
		serv_idmsg=true;
		*serv_idmsg_type=*text_crypted_v;
		++text_crypted_v;
		
		// try again to see if it's a crypted message
		goto msg_iscrypt_match;
	}
	else {
		// not crypted
		goto exit_eat_none;
	}

	// try to decrypt it
	if(!mc_decrypt_text(text_decrypted_buf,text_crypted_v,key_buf))
		goto exit_eat_none;

	// re-issue the command with the decrypted text
	GLOBAL__TEXT_WAS_CRYPTED = true;

	// now here comes the trickery, killing all 4 cases in one go
	xchat_commandf(ph,"recv %s %s %s :%s%s%s%s"
			, 			  word[RP_SOURCE]
			, 			  word[RP_CMD]
			, 			  word[RP_TARGET]
			, (serv_idmsg 		? serv_idmsg_type 	: "")
			, (msg_type_action 	? "\001ACTION " 	: "")
			, 			  text_decrypted_buf
			, (msg_type_action 	? "\001" 		: ""));

	// by the time xchat_commandf returns the raw handler was called a second time and the text event handler was also called
	// we now HAVE to return EAT_ALL to prevent confusion
	GLOBAL__TEXT_WAS_CRYPTED = false;

	return EAT_ALL;
exit_eat_none:
	return EAT_NONE;
}


/// \brief try to decrypt an arbitrary string of text, if enabled it will also strip all mirc colorcodes from the resulting text
/// \author Gregor Jehle <gjehle@gmail.com>
/// \param text_decrypted buffer of atleast size MAXRETURNSTRINGLEN to hold result data
/// \param text_crypted a text string that should be decrypted
/// \param key the key that should be used to decrypt
/// \return true on success, false on failure
bool mc_decrypt_text(char *text_decrypted, const char *text_crypted, const char* key)
{
	bool bretv;
	bretv = mircryptor->mc_decrypt2((char*)key,(char*)text_crypted,text_decrypted);
	if(!bretv) {
		if( strcmp(text_decrypted,text_crypted) != 0 && text_decrypted[0] != '\0' )
			xchat_print(ph,text_decrypted);
		return false;
	}
	else if( strcmp(text_decrypted,text_crypted) != 0 && text_decrypted[0] != '\0' ) {
		strip_mirc_colors(ph,text_decrypted);
		return true;
	}
	return false;
}


/// \brief extract nickname from a standard format complete hostmask
/// \author Gregor Jehle <gjehle@gmail.com>
/// \param nick buffer to copy the nickname to, must be atleast MAXCHANNELNAMESIZE big
/// \param mask mask in the format "nick!ident@host"
/// \return true on success, false on failure
static bool mc_get_nick_from_mask(char *nick, const char *mask)
{
	char buf[MAXLINELEN];
	mclinesafe_strcpy(buf,mask);
	char* token = strtok(buf,"!");
	if(token == NULL) return false;
	else mcnicksafe_strcpy(nick,token);
	return true;
}

/// \brief one-stop handle all text events that might be caused by re-issued decrypted messages
/// \author Gregor Jehle <gjehle@gmail.com>
/// \param word see xchat documentation
/// \param word_eol see xchat documentation
/// \param userdata see xchat documentation
/// \return true on success, false on failure
static int mc_grand_unified_eventhandler(char *word[], void *userdata)
{
	// if that's false, then it's a text event for text that wasn't crypted
	if(!GLOBAL__TEXT_WAS_CRYPTED)
		return EAT_NONE;

	// NOTICE: ORDER MATTERS! do not try to match "foo" before "foobar", the second match will NEVER happen!

	// modify it!
	if( strncmp(word[0],"Channel Message",15) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeChannelMessage],word[1],word[2]);

	else if( strncmp(word[0],"Channel Msg Hilight",19) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeChannelMsgHilight],word[1],word[2]);

	//else if( strncmp(word[0],"Your Message",12) == 0 )
	//	xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeYourMessage],word[1],word[2]);

	else if( strncmp(word[0],"Channel Action Hilight",22) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeChannelActionHilight],word[1],word[2]);

	else if( strncmp(word[0],"Channel Action",14) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeChannelAction],word[1],word[2]);

	//else if( strncmp(word[0],"Your Action",11) == 0 )
	//	xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeYourAction],word[1],word[2]);

	//else if( strncmp(word[0],"Notice Send",11) == 0 )
	//	xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeNoticeSend],word[1],word[2]);

	else if( strncmp(word[0],"Notice",6) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeNotice],word[1],word[2]);

	else if( strncmp(word[0],"Channel Notice",14) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeChannelNotice],word[1],word[2],word[3]);

	//else if( strncmp(word[0],"Message Send",12) == 0 )
	//	xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeMessageSend],word[1],word[2]);

	else if( strncmp(word[0],"Private Message to Dialog",25) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTePrivateMessageToDialog],word[1],word[2]);

	else if( strncmp(word[0],"Private Message",15) == 0 )
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTePrivateMessage],word[1],word[2]);

	//else if( strncmp(word[0],"Topic",) == 0 )
	//else if( strncmp(word[0],"Topic Creation",) == 0 )
	//else if( strncmp(word[0],"Topic Change",) == 0 )
	else {
		xchat_printf(ph,"MCPS: ERROR: unkown text event captured: %s",word[0]);
		return EAT_NONE;
	}
	return EAT_ALL;
}

//---------------------------------------------------------------------------
/// \brief this function tries to migrate the key if a user changes nicknames
/// \author Gregor Jehle <gjehle@gmail.com>
/// \param word see xchat documentation
/// \param userdata see xchat documentation
/// \return EAT_ALL or EAT_NONE (see xchat documentation)
static int mc_event_changenick(char *word[], void *userdata)
{
	char res[MAXRETURNSTRINGLEN];

	char key1[MAXKEYSIZE];
	char key2[MAXKEYSIZE];
	
	char nick1[MAXCHANNELNAMESIZE];
	char nick2[MAXCHANNELNAMESIZE];

	mcnicksafe_strcpy(nick1,word[1]);
	mcnicksafe_strcpy(nick2,word[2]);
	mylowercasify(nick1);
	mylowercasify(nick2);

	mircryptor->mc_displaykey(nick1,key1);
	mircryptor->mc_displaykey(nick2,key2);
	
	// we don't have a key to begin with
	if(*key1 == '\0') return EAT_NONE;

	// already a DIFFERENT key set for the new name
	if(*key2 != '\0' && strcmp(key1,key2)!=0) {
		xchat_printf(ph,"MCPS: WARNING: tried to migrate key for %s to %s, "
				"but %s already has a different key set!\n",word[1],word[2],word[2]);
	}
	// migrate
	else if(*key2 == '\0') {
		mircryptor->mc_setkey(nick2,key1,res);
		xchat_printf(ph,"MCPS: Keymigration result: %s\n",res);
	}

	return EAT_NONE;
}


static int mc_event_topic(char *word[], void *userdata)
{
	// handle event
	char returndata[MAXRETURNSTRINGLEN];
	bool bretv;

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,word[1]);
	// new force lowercase
	mylowercasify(channelname);

	char text[MAXLINELEN];
	mclinesafe_strcpy(text,word[2]);
	char mynick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(mynick,xchat_get_info(ph, "nick"));
	char server[255];
	mcsafe_strcpy(server,xchat_get_info(ph, "server"),255);

	// decrypt it
	bretv=mircryptor->mc_decrypt2(channelname,text,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
			xchat_print(ph,returndata);
		return EAT_NONE;
		}
	if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// it was decrypted so we handle it
		// we dont need to print it since it will be displayed automatically
		// xchat_printf(ph,"Topic for %s is decrypted as: %s",channelname,returndata);
		strip_mirc_colors(ph,returndata);
		xchat_commandf(ph,"recv :%s 332 %s %s :(e) %s",server,mynick,channelname,returndata);
		return EAT_ALL;
		}

	// don't eat this event, let xchat handle it also
	return EAT_NONE;
}

static int mc_event_topicchange(char *word[], void *userdata)
{
	// handle event
	char returndata[MAXRETURNSTRINGLEN];
	bool bretv;

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	char nick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(nick,word[1]);
	char text[MAXLINELEN];
	mclinesafe_strcpy(text,word[2]);

	// decrypt it
	bretv=mircryptor->mc_decrypt2(channelname,text,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
			xchat_print(ph,returndata);
		return EAT_NONE;
		}
	if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// it was decrypted so we handle it
		// no need to annnounce it, it will get announced automatically by xchat
		// xchat_printf(ph,"%s has encrypted the topic for %s to: %s",nick,channelname,returndata);
		strip_mirc_colors(ph,returndata);
		xchat_commandf(ph,"recv :%s!%s@mircryption TOPIC %s :(e) %s",nick,nick,channelname,returndata);
		return EAT_ALL;
		}

	//xchat_printf(ph,"TOPICCHANGE nick='%s' chjannel='%s' text='%s' return='%s'",nick,channelname,text,returndata);

	// don't eat this event, let xchat handle it also
	return EAT_NONE;
}

static int mc_event_yourmessage(char *word[], void *userdata)
{
	// issue a command to xchat
	// this is kind of wierd because SOMETIMES encryption is handled prior to this in mc_alltext()
	char returndata[MAXRETURNSTRINGLEN];
	bool bretv;

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	char nick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(nick,word[1]);
	char text[MAXLINELEN];
	mclinesafe_strcpy(text,word[2]);

	if (strncmp(text,"mcps ",5)==0)
		return EAT_ALL;
	if (strncmp(text,OUTGOINGETAG,strlen(OUTGOINGETAG))==0)
		return EAT_ALL;

	// don't eat this event, let xchat handle it also
	return EAT_NONE;
}

static int mc_event_youraction(char *word[], void *userdata)
{
	char decrypted_buf[MAXRETURNSTRINGLEN];
	char target[MAXCHANNELNAMESIZE];
	char nick[MAXCHANNELNAMESIZE];
	char crypted_buf[MAXLINELEN];

	mcnicksafe_strcpy(target,xchat_get_info(ph, "channel")); 
	mylowercasify(target);

	mcnicksafe_strcpy(nick,word[1]);

	mclinesafe_strcpy(crypted_buf,word[2]);

	//lets see if we can decrypt the notice we just sent
	if(!mc_decrypt_text(decrypted_buf,crypted_buf,target)) {
		//if (*decrypted_buf != '\0') {
		//	xchat_print(ph,decrypted_buf);
		//	return EAT_ALL;
		//} else {
		//	return EAT_NONE;
		//}
		return EAT_NONE;
	} else if (strcmp(decrypted_buf,crypted_buf)!=0 && decrypted_buf[0]!='\0') {
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeYourAction],nick,decrypted_buf);
		return EAT_ALL;
	}
	return EAT_NONE;
}

static int mc_event_noticesend(char *word[], void *userdata)
{
	char returndata[MAXRETURNSTRINGLEN];
	bool bretv;
	
	// get info
	char dest[MAXCHANNELNAMESIZE];
	// gjehle 070328: new xchat version no longer gives destination in word[1]
	// mcnicksafe_strcpy(dest,word[1]);
	// the nickname is now sent in word[1]
	mcnicksafe_strcpy(dest,xchat_get_info(ph, "channel")); 
	mylowercasify(dest);
	char nick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(nick,word[1]);

	char text[MAXLINELEN];
	mclinesafe_strcpy(text,word[2]);

	// if its a meow reply then its already displayed by us so we do nothing here
	if (strncmp(text,"mcps meow meowreply",19)==0)
		return EAT_ALL;

	//lets see if we can decrypt the notice we just sent

	bretv=mircryptor->mc_decrypt2(dest,text,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,"")!=0)
			{
			xchat_print(ph,returndata);
			return EAT_ALL;
			}
		else
			return EAT_NONE;
		}
	else if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// gjehle 070328: now we have to use 'nick' instead of 'dest'
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeNoticeSend],nick,returndata);
		return EAT_ALL;
		}

	// don't eat this event, let xchat handle it also
	return EAT_NONE;

}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
static int mc_help(char *word[], char *word_eol[], void *userdata)
{
	xchat_print(ph,"-\n");
	xchat_print(ph,". Mircryption - xchat version - Dark Raichu & Xro\n");
	xchat_print(ph,". blowfish encryption algorithms are by bruce schneier and jim conger.\n");
	xchat_print(ph,".... /setkey [keyphrase]'             enables encryption/decryption on specified channel (defaults to current channel), using key specified.  can be used to add or modify keys.\n");
	xchat_print(ph,".... /delkey [channel]'               removes encryption key for specified channel (defaults to current channel).\n");
	xchat_print(ph,".... /disablekey'                     temporarily disables encryption for current channel\n");
	xchat_print(ph,".... /enablekey'                      re-enables encryption for current channel\n");
	xchat_print(ph,".... /displaykey'                     shows you (and only you) the key for the current channel\n");
	//      xchat_print(ph,".... /plain ...'                      send ... text without encryption\n");
	xchat_print(ph,".... /listkeys'                       lists all channel encryption keys currently stored\n");
	xchat_print(ph,"... /keypassphrase [phrase]'        set or change current master keyfile passphrase to keyword\n");
	//      xchat_print(ph,".... /mcscheme X'                     set color/identification scheme, where X is the number 1-3\n");
	xchat_print(ph,".... /etopic [text]'                  set an encrypted topic for the channel.\n");
	//      xchat_print(ph,".... /encryptecho channelname text'   echoes encrypted version of text IF channel is set to encrypt, otherwise plaintext\n");
	//      xchat_print(ph,".... /decryptecho channelname text'   echoes decrypted version of text IF channel has a key, otherwise plaintext\n");
	xchat_print(ph,".... /setkeyfile filename'            set the name of the file to be used for storing/retrieving keys\n");
	//      xchat_print(ph,".... /emsg channelname test...        replacement for /msg - encrypts text if appropriate (good for bots, etc.)\n");
	//      xchat_print(ph,".... /mcmeow [channelname]            broadcast handshake query to channel\n");
	//      xchat_print(ph,".... /etext text...                   same as typing text... in encrypted channel, BUT igores disabling, and wont send to channel without encryption\n");
	//      xchat_print(ph,".... /textpad                         launches the textpad dialog for copy and paste of big text\n");
	xchat_print(ph,"-\n");

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
static int mc_setkey(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char keyphrase[MAXKEYSIZE];
	mckeysafe_strcpy(keyphrase,word_eol[2]); 
	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	// execute command
	mircryptor->mc_setkey(channelname,keyphrase,returndata);

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_delkey(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	// execute command
	mircryptor->mc_delkey(channelname,returndata);

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_disablekey(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	// execute command
	mircryptor->mc_disablekey(channelname,returndata);

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_enablekey(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	// execute command
	mircryptor->mc_enablekey(channelname,returndata);

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_displaykey(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	// execute command
	mircryptor->mc_displaykey(channelname,returndata);

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_listkeys(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// execute command
	mircryptor->mc_listkeys(returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_keypassphrase(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char keyphrase[MAXKEYSIZE];
	strcpy(keyphrase,word_eol[2]); 

	// execute command
	mircryptor->mc_setunlockpassphrase(keyphrase,returndata);

	// no matter if the result is good or bad, we already executed the masterkey command atleast once
	// remember this to prevent the passphrase getstr dialog to pop up
	GLOBAL__MASTERKEY_SET = true;

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_setkeyfile(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char keyfile[MAXKEYSIZE];
	strcpy(keyfile,word_eol[2]); 

	// execute command
	mircryptor->mc_setkeyfilename(keyfile,returndata);

	// display result
	xchat_printf(ph, "%s",returndata);

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_etopic(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	bool bretv;
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char text[MAXLINELEN];
	char text2[MAXLINELEN];
	strcpy(text,word_eol[2]); 
	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	// utf8 fix for text
	Utf8DirtyFix(text,text2);

	// encrypt it
	bretv=mircryptor->mc_encrypt2(channelname,text2,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,"")!=0)
			{
			xchat_print(ph,returndata);
			return EAT_ALL;
			}
		else
			return EAT_NONE;
		}
	else if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// it was encrypted so we handle it
		// send the encrypted text
		xchat_commandf(ph,"TOPIC %s %s",channelname,returndata);
		// we dont actually display it here, as it will be displayed on the event we trigger when we set the topic
		// xchat_printf(ph,"Encrypted topic for [%s] set to: %s",channelname,text);
		return EAT_ALL;
		}
	else
		{
		// error
		xchat_printf(ph,"topic could not be encrypted, so it wasnt set.");
		}

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}


static int mc_alltext(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	bool bretv;
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	char text[MAXLINELEN];
	char text2[MAXLINELEN];
	mclinesafe_strcpy(text,word_eol[1]);
	char nick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(nick,xchat_get_info(ph, "nick")); 

	// note we dont need this (in fact it causes problems) because xchat, unlike mirc
	//  will not invoke mc_alltext on commands
	/*
	if (word_eol[1][0]=='/')
		{
		// a command, so not for us
		return EAT_NONE;
		}
	*/

	// are they using the reverse-encryption prefix?
	bool isencrypting=mircryptor->mc_isencrypting(channelname,returndata);
	if (text[0]=='`')
		{
		// yes they are using the prefix.  remove it.
		strcpy(text,&text[1]);
		if (isencrypting)
			{
			// they are normally set to encrypt, so we disable it and send plain text
			xchat_commandf(ph,"MSG %s %s",channelname,text);
			// we dont show the user what they typed here, it will be shown in event_ourmessage
			//	xchat_printf(ph,MESSAGE_SEND_FORMAT,nick,text);
			return EAT_ALL;
			}
		// encryption is disabled normally (or doesnt exist at all)
		// we just drop through in this case.
		}
	else if (!isencrypting)
		return EAT_NONE;

	// utf8 fix for text
	Utf8DirtyFix(text,text2);

	// encrypt it
	bretv=mircryptor->mc_forceencrypt(channelname,text2,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,"")!=0)
			{
			xchat_print(ph,returndata);
			return EAT_ALL;
			}
		else
			return EAT_NONE;
		}
	else if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// it was encrypted so we handle it
		// send the encrypted text
		xchat_commandf(ph,"MSG %s %s %s",channelname,OUTGOINGETAG,returndata);
		// show user their own text
		//xchat_printf(ph,"[%s] %s",nick,text);
		//xchat_commandf(ph,"recv :%s!%s@mircryption PRIVMSG %s :(e) %s",nick,nick,channelname,text);
		xchat_printf(ph,(char*)TEXT_EVENT_FORMAT[eTeYourMessage],nick,text);
		return EAT_ALL;
		}

	// let xchat handle this normally
	return EAT_NONE;
}



static int mc_action(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	bool bretv;
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char text[MAXLINELEN];
	char text2[MAXLINELEN];
	strcpy(text,word_eol[2]); 
	// get info
	char channelname[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(channelname,xchat_get_info(ph, "channel")); 
	// new force lowercase
	mylowercasify(channelname);

	char nick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(nick,xchat_get_info(ph, "nick")); 
	
	if (strcmp(word[2],"mcps")==0 || strcmp(word[2],OUTGOINGETAG)==0)
		{
		//we already encrypted it
		//even though this is a not-nice hack
		//it is necessary to avoid recursive calls
		return EAT_NONE;
		}

	// utf8 fix for text
	Utf8DirtyFix(text,text2);

	// encrypt it
	bretv=mircryptor->mc_encrypt(channelname,text2,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,"")!=0)
			{
			xchat_print(ph,returndata);
			return EAT_ALL;
			}
		else
			return EAT_NONE;
		}
	else if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// it was encrypted so we handle it
		// send the encrypted text
		xchat_commandf(ph,"me %s %s",OUTGOINGETAG,returndata);
		//xchat_printf(ph,"mc \00313*\00300\t%s %s",nick,text); //xchat already does that when the action comes back from the server
		return EAT_ALL;
		}
	else
		{
		// error
		xchat_printf(ph,"action could not be encrypted, so it wasnt set.");
		}

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}

static int mc_notice(char *word[], char *word_eol[], void *userdata)
{
	// handle this command
	bool bretv;
	char returndata[MAXRETURNSTRINGLEN];

	// get info
	char text[MAXLINELEN];
	char text2[MAXLINELEN];
	mclinesafe_strcpy(text,word_eol[3]); 
	// get destination (nick/channel)
	char dest[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(dest,word[2]); 
	// new force lowercase
	mylowercasify(dest);

	if (strcmp(word[3],"mcps")==0 || strcmp(word[3],OUTGOINGETAG)==0)
		{
		//we already encrypted it
		//even though this is a not-nice hack
		//it is necessary to avoid recursive calls
		return EAT_NONE;
		}

	// utf8 fix for text
	Utf8DirtyFix(text,text2);

	// encrypt it
	bretv=mircryptor->mc_encrypt(dest,text2,returndata);
	if (!bretv)
		{
		if (strcmp(returndata,"")!=0)
			{
			xchat_print(ph,returndata);
			return EAT_ALL;
			}
		else
			return EAT_NONE;
		}
	else if (strcmp(returndata,text)!=0 && returndata[0]!='\0')
		{
		// it was encrypted so we handle it
		// send the encrypted text
		xchat_commandf(ph,"notice %s %s %s",dest,OUTGOINGETAG,returndata);
		//xchat_printf(ph,TEXT_EVENT_FORMAT[eTeNoticeSend],dest,text2);
		return EAT_ALL;
		}
	else
		{
		// error
		xchat_printf(ph,"notice could not be encrypted, so it wasnt sent.");
		}

	/// eat this command so xchat and other plugins can't process it
	return EAT_ALL;
}

//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
char *MircryptionClass_xchat::get_classversionstring()
{
	// return some version information - you dont have to override this one
	return "Mircryption class for xchat";
}


bool MircryptionClass_xchat::present_messsagebox(char *messagetext,char *windowtitle)
{
	// present some info to the user, preferably in a pop-up modal dialog
	// returns true on success

	display_statustext(messagetext);
	return false;
}


// we need the user to give us a valid master passphrase
bool MircryptionClass_xchat::request_unlockpassphrase()
{
	// return true if user gives us a valid passphrase, and unlock the keys, false if not
	// I dont know how to ask xchat client for info yet, so instead we just return false and expect uuser to set it with /keypassphrase
	return false;
}


bool MircryptionClass_xchat::send_irccommand(char *irccommand,char *text)
{
	// send an irc command to server
	// returns true on success

	xchat_commandf(ph,"%s %s",irccommand,text);
	return false;
}


bool MircryptionClass_xchat::display_statustext(char *messagetext)
{
	// display some information for the user in some default text area (like status window in mirc)
	// returns true on success

	xchat_print(ph, messagetext);
	return true;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void mylowercasify(char *str)
{
	if (str==NULL)
		return;

	int len=strlen(str);
	char c;

	for (int count=0;count<len;++count)
		{
		c=str[count];
		if (c>='A' && c<='Z')
			str[count]=c+32;
		}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
/*
  Author: gjehle <gjehle@gmail.com>
  Date: 27.12.03 22:09
  Description: frontend to mirc_codes.h
*/
void strip_mirc_colors(xchat_plugin* ph, char* textline)
{
	int i;
	const char *str;
	if (xchat_get_prefs(ph,"text_stripcolor",&str, &i) == 3) // successfully got boolan value
		{
		if(i == 1) // stripping enabled
			{
			mirc_codes::clean(textline);
			}
		}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool HandleMeow(char *channelname,char *nick,char *text)
{
	// handle a meow and return true on success
	char returndata[MAXRETURNSTRINGLEN],estatus[MAXRETURNSTRINGLEN];
	bool bretv;

	// grab sender string
	char *senderstring,*inteststring;
	senderstring=strtok(text," ");
	senderstring=strtok(NULL," "); senderstring=strtok(NULL," "); senderstring=strtok(NULL," ");senderstring=strtok(NULL," ");
	inteststring=strtok(NULL," ");
		
	// if they want to ignore meows
	if (IGNOREMEOWS || senderstring==NULL || inteststring==NULL)
		{
		xchat_printf(ph,"ignoring meow broadcast from %s on channel %s (set IGNOREMEOWS to false in mircryption.cpp to stop ignoring).",nick,channelname);
		return true;
		}

	// decrypt the test string
	char teststring[MAXRETURNSTRINGLEN];
	sprintf(teststring,"mcps %s",inteststring);
	bretv=mircryptor->mc_decrypt2(channelname,teststring,returndata);
	if (!bretv || strcmp(returndata,teststring)==0 || returndata[0]=='\0')
		{
		// no key for chan
		strcpy(estatus,"no encryption key for this channel");
		}
	else
		{
		// does it match meow senders key
		if (strcmp(returndata,"meow")==0)
			strcpy(estatus,"crypting (key match)");		
		else
			strcpy(estatus,"crypting (key mismatch)");
		}

	// get our nick
	char mynick[MAXCHANNELNAMESIZE];
	mcnicksafe_strcpy(mynick,xchat_get_info(ph, "nick")); 
	
	// show user what we are replying
	xchat_printf(ph,"[=^.^=] [%s] %s -> meow %s %s",senderstring,nick,channelname,estatus);

	// send reply as notice
	sprintf(returndata,"mcps meow meowreply %s %s [%s] %s -> %s",nick,channelname,XMC_VERSIONSTRING,mynick,estatus);
	xchat_commandf(ph,"notice %s %s",nick,returndata);
	
	// return success
	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void Utf8DirtyFix(char *intext,char *outtext)
{
	// walk through the string and change any xchat encoded utf8 (glib) special characters
	// really the ideal way to fix this is for xchat developers to expose a function to conver the plugin-passed utf8 text to local text
	// but for now we use this quick fix
	unsigned char c,c2;
	int len=strlen(intext);
	int desti=0;
	
	for (int count=0;count<len;++count)
	    {
		c=intext[count];
		if (c==195)
		    {
			// ok special two-byte character prefix (skip over it)
			++count;
			c2=intext[count];
			// now set c to actual character to replace
			if (c2==0)
				c=0;
			else
			    {
			    // apparent dirty fix for utf8
				c=c2+64;
				}
			}
		// write it and increment index
		outtext[desti]=c;
		++desti;
		}
	// '\0' terminate
	outtext[desti]='\0';
}
//---------------------------------------------------------------------------
