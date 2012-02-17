//
// Copyright 2010-2011 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//


/**
  @author: modified by lzyou@ie.cuhk.edu.hk
  @date  : Feb. 17, 2012
  @func  : Tx Single File Bursts
*/

#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>
#include <csignal>
#include <iostream>
#include <fstream>
#include <complex>

namespace po = boost::program_options;

static bool stop_signal_called = false;
void sig_int_handler(int){stop_signal_called = true;}

int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args, file;
    double seconds_in_future;
    size_t total_num_samps, total_num_pkts;
    double rate;
//    float ampl;
    double freq;
    double rep_rate;
    double gain;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi uhd device address args")
        ("file", po::value<std::string>(&file)->default_value("ofdm_zerogap.dat"), "name of the file to read binary samples from")
        ("secs", po::value<double>(&seconds_in_future)->default_value(1.5), "delay before first burst")
        ("repeat", "repeat burst")
        ("rep-delay", po::value<double>(&rep_rate)->default_value(0.5), "delay between bursts")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(10000), "total number of samples to transmit")
        ("rate", po::value<double>(&rate)->default_value(100e6/16), "rate of outgoing samples")
//        ("ampl", po::value<float>(&ampl)->default_value(float(0.3)), "amplitude of each sample")
        ("freq", po::value<double>(&freq)->default_value(0), "center frequency")
        ("gain", po::value<double>(&gain)->default_value(0), "gain")
        ("npkts", po::value<size_t>(&total_num_pkts)->default_value(5), "total number of packets to transmit")
        ("dilv", "specify to disable inner-loop verbose")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("UHD TX Timed Samples %s") % desc << std::endl;
        return ~0;
    }

    bool verbose = vm.count("dilv") == 0;
    bool repeat = vm.count("repeat") != 0;

    //create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);
    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    //set the tx sample rate
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;
    usrp->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate()/1e6) << std::endl << std::endl;

    std::cout << boost::format("Setting TX Freq: %f MHz...") % (freq/1e6) << std::endl;
    for(size_t i=0; i < usrp->get_tx_num_channels(); i++) usrp->set_tx_freq(freq, i);
    std::cout << boost::format("Actual TX Freq: %f MHz...") % (usrp->get_tx_freq()/1e6) << std::endl << std::endl;

    std::cout << boost::format("Setting TX Gain: %f...") % (gain) << std::endl;
    for(size_t i=0; i < usrp->get_tx_num_channels(); i++) usrp->set_tx_gain(gain, i);
    std::cout << boost::format("Actual TX Gain: %f...") % (usrp->get_tx_gain()) << std::endl << std::endl;

    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
    usrp->set_time_now(uhd::time_spec_t(0.0));

    //create a transmit streamer
    uhd::stream_args_t stream_args("fc32"); //complex floats
    uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);

    //allocate buffer with data to send
    const size_t spb = tx_stream->get_max_num_samps();

    std::vector<std::complex<float> > buff(spb);
    if(verbose) std::cout << boost::format("Buff size: %u %u") % spb % buff.size() << std::endl;
//    std::vector<std::complex<float> *> buffs;
//    for(size_t i=0; i < usrp->get_num_mboards(); i++) {
//        buffs.push_back(new std::complex<float>[spb]);
//        for(size_t n=0; n < spb; n++)
//            buffs.back()[n] = std::complex<float>(ampl, ampl);
//    };

    std::signal(SIGINT, &sig_int_handler);
    if(repeat) std::cout << "Press Ctrl + C to quit..." << std::endl;

    double time_to_send = seconds_in_future;
    int counter = 0;

    do {
        counter++;
        //setup metadata for the first packet
        uhd::tx_metadata_t md;
        md.start_of_burst = true;
        md.end_of_burst = false;
        md.has_time_spec = true;
        md.time_spec = uhd::time_spec_t(time_to_send);
        std::cout << boost::format("Set timestamp: %u") % time_to_send << std::endl;

        //the first call to send() will block this many seconds before sending:
        double timeout = std::max(rep_rate, seconds_in_future) + 0.1; //timeout (delay before transmit + padding)
/**
        size_t num_acc_samps = 0; //number of accumulated samples
        while(num_acc_samps < total_num_samps){
            size_t samps_to_send = std::min(total_num_samps - num_acc_samps, spb);

            //ensure the the last packet has EOB set
            md.end_of_burst = samps_to_send < spb;

            //send a single packet
            size_t num_tx_samps = tx_stream->send(
                buffs, samps_to_send, md, timeout
            );
                
            //do not use time spec for subsequent packets
            md.has_time_spec = false;
            md.start_of_burst = false;

            if (num_tx_samps < samps_to_send) std::cerr << "Send timeout..." << std::endl;
            if(verbose) std::cout << boost::format("Sent packet: %u samples | EOB: %d ") % num_tx_samps % md.end_of_burst << std::endl;

            num_acc_samps += num_tx_samps;
        }

        time_to_send += rep_rate;
*/
        std::ifstream infile(file.c_str(), std::ifstream::binary);
        size_t num_acc_samps = 0; //number of accumulated samples
        //loop until the entire file has been read
        while(not md.end_of_burst){

            infile.read((char*)&buff.front(), buff.size()*sizeof(std::complex<float>));
            size_t samps_to_send = infile.gcount()/sizeof(std::complex<float>);

            md.end_of_burst = infile.eof();

            size_t num_tx_samps = tx_stream->send(&buff.front(), samps_to_send, md);

            if (num_tx_samps < samps_to_send) std::cerr << "Send timeout..." << std::endl;
            num_acc_samps += num_tx_samps;
            if(verbose) std::cout << boost::format("Sent packet: %u samples | SOB: %d | TIME: %d |EOB: %d ") % num_tx_samps % md.start_of_burst % md.has_time_spec % md.end_of_burst << std::endl;

            //do not use time spec for subsequent packets
            md.has_time_spec = false;
            md.start_of_burst = false;
        }

        infile.close();

        time_to_send += rep_rate;


        std::cout << std::endl << "Waiting for async burst ACK... " << std::flush;
        uhd::async_metadata_t async_md;
        bool got_async_burst_ack = false;
        //loop through all messages for the ACK packet (may have underflow messages in queue)
        while (not got_async_burst_ack and usrp->get_device()->recv_async_msg(async_md, seconds_in_future)){
            got_async_burst_ack = (async_md.event_code == uhd::async_metadata_t::EVENT_CODE_BURST_ACK);
        }
        std::cout << (got_async_burst_ack? "success" : "fail") << std::endl;
    } while (counter < total_num_pkts);

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return 0;
}
