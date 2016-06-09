/// This example demonstrates a bot that takes URLs from messages, fetches them
/// and displays their content in a reply to the user.
/// It also demonstrates more advanced message sending parameters.
/// This uses libvoxelstorm's automatic message splitting, for easily posting
/// long messages.

#include "libtelegram/libtelegram.h"

auto main()->int {
  std::string const token("bot123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11");
  telegram::listener listener;
  telegram::sender sender(token);
  listener.set_callback_message([&](boost::property_tree::ptree const &input){
    int_fast32_t const message_id(     input.get<int_fast32_t>("message_id", 0));
    int_fast64_t const message_chat_id(input.get<int_fast64_t>("chat.id", -1));
    std::string  const message_text(   input.get("text", ""));

    urdl::url url;
    try {
      // try to parse the text to see if it's a URL
      url = message_text;                                                       // this throws if a valid URL cannot be created
    } catch(std::exception const &e) {
      sender.send_message(message_chat_id,                                      // the chat to reply to
                          std::string("That doesn't appear to be a valid URL: ") + e.what(), // text of the message to send including the exception
                          message_id);                                          // reply to the sender's message id
      return;
    }
    try {
      urdl::istream stream(url);                                                // open a stream to get that URL
      if(!stream) {
        sender.send_message(message_chat_id,                                    // the chat to reply to
                            std::string("Error when opening that URL:") + stream.error().message(), // text of the message to send including the URDL error message
                            message_id);                                        // reply to the sender's message id
        return;
      }
      // now read the contents of the URL line by line and concatenate them into a single message
      std::stringstream page;                                                   // a stream buffer to collect the data together
      std::string line;                                                         // an intermediate line buffer
      unsigned int line_limit = 100;                                            // set a safety limit after which we stop
      while(std::getline(stream, line) && --line_limit) {                       // read the stream line by line, until EOF or we reach our line limit
        page << line << '\n';                                                   // and collect it together, inserting newlines ourselves - we don't use std::endl because it varies by platform
      }
      sender.send_message(message_chat_id,                                      // the chat to reply to
                          page.str(),                                           // send the complete collected page
                          message_id,                                           // reply to the sender's message id
                          telegram::sender::parse_mode::NONE,                   // don't attempt to render HTML elements, see https://core.telegram.org/bots/api#html-style
                          telegram::sender::web_preview_mode::DISABLE,          // don't allow URLs to produce previews
                          telegram::sender::notification_mode::DEFAULT);        // leave notifications at the API default mode
    } catch(std::exception const &e) {
      sender.send_message(message_chat_id,                                      // the chat to reply to
                          std::string("Exception when opening that URL:") + e.what(), // text of the message to send including the thrown exception
                          message_id);                                          // reply to the sender's message id
    }
  });
  listener.run();
  return EXIT_SUCCESS;
};