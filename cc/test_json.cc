#include "json.hh"
#include <fstream>

int main() {
  for (const std::u8string_view text : {
           u8"false",
           u8"null",
           u8"true",
           u8"\"Hello world!\"",
           u8"42",
       }) {
    assert(json_parser(u8iterator(text)).lex_json_text() == -1);
  }

  for (const std::filesystem::path path : {
           "Image.json",
           "San_Francisco_and_Sunnyvale.json",
       }) {
    std::basic_ifstream<char8_t> file(path, std::ios_base::binary);
    u8iterator source(std::ranges::basic_istream_view<char8_t, char8_t>{file});
    assert(json_parser(source).lex_json_text() == -1);
  }
}
