/*

Copyright (c) 2013, Project OSRM contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef API_GRAMMAR_HPP
#define API_GRAMMAR_HPP

#include <boost/bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>

namespace qi = boost::spirit::qi;

template <typename Iterator, class HandlerT> struct APIGrammar : qi::grammar<Iterator>
{
    explicit APIGrammar(HandlerT *h) : APIGrammar::base_type(api_call), handler(h)
    {
        api_call = qi::lit('/') >> string[boost::bind(&HandlerT::setService, handler, ::_1)] >>
                   -query;
        query = ('?') >> +(zoom | output | jsonp | checksum | uturns | location_with_options | destination_with_options | source_with_options |  cmp |
                            language | instruction | geometry | alt_route | old_API | num_results |
                            matching_beta | gps_precision | classify | locs);
        // all combinations of timestamp, uturn, hint and bearing without duplicates
        t_u = (u >> -timestamp) | (timestamp >> -u);
        t_h = (hint >> -timestamp) | (timestamp >> -hint);
        u_h = (u >> -hint) | (hint >> -u);
        t_u_h = (hint >> -t_u) | (u >> -t_h) | (timestamp >> -u_h);
        location_options = (bearing >> -t_u_h) | (t_u_h >> -bearing) | //
                           (u >> bearing >> -t_h) | (timestamp >> bearing >> -u_h) | (hint >> bearing >> t_u) | //
                           (t_h >> bearing >> -u) | (u_h >> bearing >> -timestamp) | (t_u >> bearing >> -hint);
        location_with_options = location >> -location_options;
        source_with_options = source >> -location_options;
        destination_with_options = destination >> -location_options;
        zoom = (-qi::lit('&')) >> qi::lit('z') >> '=' >>
               qi::short_[boost::bind(&HandlerT::setZoomLevel, handler, ::_1)];
        output = (-qi::lit('&')) >> qi::lit("output") >> '=' >>
                 string[boost::bind(&HandlerT::setOutputFormat, handler, ::_1)];
        jsonp = (-qi::lit('&')) >> qi::lit("jsonp") >> '=' >>
                stringwithPercent[boost::bind(&HandlerT::setJSONpParameter, handler, ::_1)];
        checksum = (-qi::lit('&')) >> qi::lit("checksum") >> '=' >>
                   qi::uint_[boost::bind(&HandlerT::setChecksum, handler, ::_1)];
        instruction = (-qi::lit('&')) >> qi::lit("instructions") >> '=' >>
                      qi::bool_[boost::bind(&HandlerT::setInstructionFlag, handler, ::_1)];
        geometry = (-qi::lit('&')) >> qi::lit("geometry") >> '=' >>
                   qi::bool_[boost::bind(&HandlerT::setGeometryFlag, handler, ::_1)];
        cmp = (-qi::lit('&')) >> qi::lit("compression") >> '=' >>
              qi::bool_[boost::bind(&HandlerT::setCompressionFlag, handler, ::_1)];
        location = (-qi::lit('&')) >> qi::lit("loc") >> '=' >>
                   (qi::double_ >> qi::lit(',') >>
                    qi::double_)[boost::bind(&HandlerT::addCoordinate, handler, ::_1)];
        destination = (-qi::lit('&')) >> qi::lit("dst") >> '=' >>
                   (qi::double_ >> qi::lit(',') >>
                    qi::double_)[boost::bind(&HandlerT::addDestination, handler, ::_1)];
        source = (-qi::lit('&')) >> qi::lit("src") >> '=' >>
                   (qi::double_ >> qi::lit(',') >>
                    qi::double_)[boost::bind(&HandlerT::addSource, handler, ::_1)];
        hint = (-qi::lit('&')) >> qi::lit("hint") >> '=' >>
               stringwithDot[boost::bind(&HandlerT::addHint, handler, ::_1)];
        timestamp = (-qi::lit('&')) >> qi::lit("t") >> '=' >>
               qi::uint_[boost::bind(&HandlerT::addTimestamp, handler, ::_1)];
        bearing = (-qi::lit('&')) >> qi::lit("b") >> '=' >>
               (qi::int_ >> -(qi::lit(',') >> qi::int_ | qi::attr(10)))[boost::bind(&HandlerT::addBearing, handler, ::_1, ::_2, ::_3)];
        u = (-qi::lit('&')) >> qi::lit("u") >> '=' >>
            qi::bool_[boost::bind(&HandlerT::setUTurn, handler, ::_1)];
        uturns = (-qi::lit('&')) >> qi::lit("uturns") >> '=' >>
                 qi::bool_[boost::bind(&HandlerT::setAllUTurns, handler, ::_1)];
        language = (-qi::lit('&')) >> qi::lit("hl") >> '=' >>
                   string[boost::bind(&HandlerT::setLanguage, handler, ::_1)];
        alt_route = (-qi::lit('&')) >> qi::lit("alt") >> '=' >>
                    qi::bool_[boost::bind(&HandlerT::setAlternateRouteFlag, handler, ::_1)];
        old_API = (-qi::lit('&')) >> qi::lit("geomformat") >> '=' >>
                  string[boost::bind(&HandlerT::setDeprecatedAPIFlag, handler, ::_1)];
        num_results = (-qi::lit('&')) >> qi::lit("num_results") >> '=' >>
                      qi::short_[boost::bind(&HandlerT::setNumberOfResults, handler, ::_1)];
        matching_beta = (-qi::lit('&')) >> qi::lit("matching_beta") >> '=' >>
               qi::float_[boost::bind(&HandlerT::setMatchingBeta, handler, ::_1)];
        gps_precision = (-qi::lit('&')) >> qi::lit("gps_precision") >> '=' >>
               qi::float_[boost::bind(&HandlerT::setGPSPrecision, handler, ::_1)];
        classify = (-qi::lit('&')) >> qi::lit("classify") >> '=' >>
            qi::bool_[boost::bind(&HandlerT::setClassify, handler, ::_1)];
        locs = (-qi::lit('&')) >> qi::lit("locs") >> '=' >>
            stringforPolyline[boost::bind(&HandlerT::getCoordinatesFromGeometry, handler, ::_1)];

        string = +(qi::char_("a-zA-Z"));
        stringwithDot = +(qi::char_("a-zA-Z0-9_.-"));
        stringwithPercent = +(qi::char_("a-zA-Z0-9_.-") | qi::char_('[') | qi::char_(']') |
                              (qi::char_('%') >> qi::char_("0-9A-Z") >> qi::char_("0-9A-Z")));
        stringforPolyline = +(qi::char_("a-zA-Z0-9_.-[]{}@?|\\%~`^"));
    }

    qi::rule<Iterator> api_call, query, location_options, location_with_options, destination_with_options, source_with_options, t_u, t_h, u_h, t_u_h;
    qi::rule<Iterator, std::string()> service, zoom, output, string, jsonp, checksum, location, destination, source,
        hint, timestamp, bearing, stringwithDot, stringwithPercent, language, geometry, cmp, alt_route, u,
        uturns, old_API, num_results, matching_beta, gps_precision, classify, locs, instruction, stringforPolyline;

    HandlerT *handler;
};

#endif /* API_GRAMMAR_HPP */
