/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 04 Aug 2016 09:08:09 AM CST
 File Name: my_capture.c
 Description: 
 ************************************************************************/

#include <alsa/asoundlib.h>

void dump_period_info(snd_pcm_hw_params_t* hw_params)
{

    snd_pcm_uframes_t approx_period_size;
    unsigned int approx_period_time;
    unsigned int rate_num, rate_den;

    /* what is `dir` used for in `get` APIs */
    /*
    int dir;
    for (dir = -1; dir < 2; dir++)
    {
        snd_pcm_hw_params_get_period_size(hw_params, &approx_period_size, &dir);
        fprintf(stdout, "Approximate period size: %ld Dir: %d\n", (long)approx_period_size, dir);

        snd_pcm_hw_params_get_period_time(hw_params, &approx_period_time, &dir);
        fprintf(stdout, "Approximate period time: %ld Dir: %d\n", (long)approx_period_time, dir);
    }
    */
    fprintf(stdout, "\n");

    snd_pcm_hw_params_get_rate_numden(hw_params, &rate_num, &rate_den);
    fprintf(stdout, "Rate numerator: %d; Rate denominator: %d Actual Rate: %f\n", rate_num, rate_den, rate_num/(double)rate_den);

    snd_pcm_hw_params_get_period_size(hw_params, &approx_period_size, NULL);
    fprintf(stdout, "Approximate period size: %ld\n", (long)approx_period_size);

    snd_pcm_hw_params_get_period_time(hw_params, &approx_period_time, NULL);
    fprintf(stdout, "Approximate period time: %ld\n", (long)approx_period_time);

    fprintf(stdout, "\n");
}

int prepare_device(const char* device_name, snd_pcm_t **handle)
{
    /* stream type */
    snd_pcm_stream_t stream     = SND_PCM_STREAM_CAPTURE;

    /* HW Params */
    snd_pcm_hw_params_t* hw_params;
    // - access type
    snd_pcm_access_t access     = SND_PCM_ACCESS_RW_INTERLEAVED;
    // - stream param
    snd_pcm_format_t format     = SND_PCM_FORMAT_S16_LE;
    unsigned int channel        = 2;
    unsigned int rate           = 44100;
    // - buffer param
    unsigned int periods        = 2;
    unsigned int period_time    = 100000; // us == 100(ms)
    unsigned int buffer_time    = 3 * period_time;

    /* SW Params */
    snd_pcm_sw_params_t* sw_params;

    int err;
    snd_output_t* snd_out;  // used for dumping PCM info

    /* Open PCM device */
    err = snd_pcm_open(handle, device_name, stream, 0);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_open failed: %s\n", snd_strerror(err));
        return 1;
    }

    /* Set HW Params */
    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_malloc failed: %s\n", snd_strerror(err));
        return 1;
    }

    err = snd_pcm_hw_params_any(*handle, hw_params);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_any failed: %s\n", snd_strerror(err));
        return 1;
    }

    // - access type
    err = snd_pcm_hw_params_set_access(*handle, hw_params, access);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_access failed: %s\n", snd_strerror(err));
        return 1;
    }

    // - stream param
    err = snd_pcm_hw_params_set_format(*handle, hw_params, format);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_format failed: %s\n", snd_strerror(err));
        return 1;
    }

    unsigned int channel_expect = channel;
    err = snd_pcm_hw_params_set_channels_near(*handle, hw_params, &channel);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_channels_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (channel != channel_expect)
    {
        fprintf(stderr, "WARN:HW:channel:\n\tExpect: %d\n\tActual: %d\n", channel_expect, channel);
    }

    unsigned int rate_expect = rate;
    err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &rate, NULL);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_rate_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (rate != rate_expect)
    {
        fprintf(stderr, "WARN:HW:rate:\n\tExpect: %d\n\tActual: %d\n",  rate_expect, rate);
    }

    // - buffer param

    unsigned int period_time_expect = period_time;
    err = snd_pcm_hw_params_set_period_time_near(*handle, hw_params, &period_time, NULL);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_period_time_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (period_time != period_time_expect)
    {
        fprintf(stderr, "WARN:HW:period_time:\n\tExpect: %d\n\tActual: %d\n", period_time_expect, period_time);
    }

    unsigned int periods_expect = periods;
    err = snd_pcm_hw_params_set_periods_near(*handle, hw_params, &periods, NULL);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_periods_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (periods != periods_expect)
    {
        fprintf(stderr, "WARN:HW:periods:\n\tExpect: %d\n\tActual: %d\n", periods_expect, periods);
    }

    unsigned int buffer_time_expect = buffer_time;
    err = snd_pcm_hw_params_set_buffer_time_near(*handle, hw_params, &buffer_time, NULL);
    if (err < 0)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_buffer_time_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (buffer_time != buffer_time_expect)
    {
        fprintf(stderr, "WARN:HW:buffer_time:\n\tExpect: %d\n\tActual: %d\n", buffer_time_expect, buffer_time);
    }

    // - set params
    err = snd_pcm_hw_params(*handle, hw_params);
    if (err < 0)
    {
        fprintf(stderr, "snd_pcm_hw_paramsfailed: %s\n", snd_strerror(err));
        return 1;
    }
    else
    {
        fprintf(stdout, "Set HW parameters finished\n");
    }

    /* Set SW Params */
    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_sw_params_malloc failed: %s\n", snd_strerror(err));
        return 1;
    }

    /* Dump PCM information */
    err = snd_output_stdio_attach(&snd_out, stdout, 0);
    if (err < 0)
    {
        fprintf(stderr, "snd_output_stdio_attach failed: %s\n", snd_strerror(err));
        return 1;
    }
    // - hw info
    fprintf(stdout, "\n- HW Params -\n");
    snd_pcm_hw_params_dump(hw_params, snd_out);
    fprintf(stdout, "\n- SW Params -\n");
    snd_pcm_sw_params_dump(sw_params, snd_out);

    snd_pcm_hw_params_free(hw_params);

    return 0;
}

int main()
{
    const char* device_name = "hw:0,0";
    //const char* device_name = "sd_carplay_downlink_in";
    int ret;
    snd_pcm_t* handle;

    ret = prepare_device(device_name, &handle);
    if (ret != 0)
    {
        fprintf(stderr, "prepare_device failed!\n");
    }
}
