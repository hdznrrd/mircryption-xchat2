# Mircryption - XChat2
A modified Micryption Plugin for XChat2 feat. DH1080 Keyexchange

- [Website](http://voobar.follvalsch.de/mcpsx)
- [Official Micryption website](http://www.donationcoder.com/Software/Mouser/mircryption/index.php)
- [Official Micryption forum](http://www.donationcoder.com/Forums/bb/index.php?board=13.0)

# History
## v0.3.3 - 04/17/07
 - gjehle: previous change messed up ACTION handling, fixed it and cleaned up the code
 - gjehle: changed masterkey input box delay to 1.5 seconds

## v0.3.2 - 04/16/07
 - gjehle: added support for idmsg servers (like freenode) who prepend messages with a + or -
           (thanks to Waky for reporting this bug)

## v0.3.1 - 04/09/07
 - gjehle: added a delayed popup to ask for the masterkey if not already entered

## v0.3.0 - 04/06/07 [official announcement]
 - gjehle: Now decrypting text in RAW messages to prevent accidential nicknighlighting
           (thanks to bt for reporting this bug)
 - gjehle: Rewrote quite some of the inbound message parser to use raw and /recv-reissue w/ textevents
           [click here for a diagram showing how the new parser works]
 - gjehle: nickhighlighting now works if text was encrypted, too
 - gjehle: Added key-migration to support nickchanges while having an encrypted converstation
 - gjehle: moved all xchat_printf template-formats to a global stringarray that can be accessed
           using enum eTextEvents
 - gjehle: began commenting of code in doxygen syntax

## v0.2.0 - 03/29/07
 - gjehle: We now obey XChat's foreground color settings for nicknames displayed when in crypto mode
 - gjehle: Event "Notice Send" got replaced by "Your Notice", updated event hook & handler
 - gjehle: Added EACTION_RECV_FORMAT and EACTION_SEND_FORMAT to replace the previous one-fits-all

## older entries:
 - 01/08/04 added new cbc blowfish modes
 - 12/26/04 fixing for xchat umlaout bug (utf8 stuff)
 - 05/18/04 added meow support
 - 03/30/04 changed default outgoing tag to +OK for better default compatibility with other scripts
 - 03/21/04 help message was saying to call '/setkey #chan key' instead of '/setkey key'
 - 12/27/03 gjehle: added color stripping
 -  9/08/03 added support for ` prefix toggle
 -  5/13/03 added windows compatibility (tested with xygwin and xchat2 for win32)
 -  5/09/03 changed default keyfile location to ~/.xchat/MircryptionKeys.txt
 -  4/28/03 added \017 characters to format, which allows nicks to be clicked.
 -  4/27/03 fixed lines with / not being encrypted
