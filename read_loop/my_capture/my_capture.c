/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 04 Aug 2016 09:08:09 AM CST
 File Name: my_capture.c
 Description: 
 ************************************************************************/

#include <alsa/asoundlib.h>
#include <signal.h>

#define VERBOSE_LOG

/****************************
 * Global Variables
 ****************************/

/* HW Param */
// - access type
snd_pcm_access_t access_type                = SND_PCM_ACCESS_RW_INTERLEAVED;
// - stream param
snd_pcm_format_t format                     = SND_PCM_FORMAT_S16_LE;
unsigned int channel                        = 2;
unsigned int rate                           = 44100;
// - buffer param
unsigned int periods                        = 4;
unsigned int period_time                    = 10000; // us == 100(ms)

/* SW Param */
unsigned int start_threshold_factor         = 0;

/* Capture toggle */
volatile sig_atomic_t signal_pause_switch   = 1;

/****************************
 * Helper Functions
 ****************************/

void pr_error(const char* msg, int err)
{
    fprintf(stderr, "%s: %s\n", msg, snd_strerror(err));
}

void dump_areainfo(const snd_pcm_channel_area_t* areas)
{
    fprintf(stdout, "Area Info:\n");
    fprintf(stdout, "Base address: %p\n", areas->addr);
    fprintf(stdout, "Offset to first sample: %u(bit)\n", areas->first);
    fprintf(stdout, "Sample distance: %u(bit)\n", areas->step);
}

void dump_period_info(snd_pcm_hw_params_t* hw_params)
{

    snd_pcm_uframes_t approx_period_size;
    unsigned int approx_period_time, approx_rate;
    unsigned int rate_num, rate_den;

    /* what is `dir` used for in `get` APIs??? */
    int dir;

    snd_pcm_hw_params_get_rate(hw_params, &approx_rate, &dir);
    fprintf(stdout, "Approximate rate: %ld", (long)approx_rate);
    switch (dir)
    {
        case (-1):
            fputs(", the exact value < this approximate value\n", stdout);
            break;
        case (0):
            fputs(", the exact value = this approximate value\n", stdout);
            break;
        case (1):
            fputs(", the exact value > this approximate value\n", stdout);
            break;
    }
    snd_pcm_hw_params_get_rate_numden(hw_params, &rate_num, &rate_den);
    fprintf(stdout, "Rate numerator: %d; Rate denominator: %d Actual Rate: %f\n", rate_num, rate_den, rate_num/(double)rate_den);

    snd_pcm_hw_params_get_period_size(hw_params, &approx_period_size, &dir);
    fprintf(stdout, "Approximate period size: %ld", (long)approx_period_size);
    switch (dir)
    {
        case (-1):
            fputs(", the exact value < this approximate value\n", stdout);
            break;
        case (0):
            fputs(", the exact value = this approximate value\n", stdout);
            break;
        case (1):
            fputs(", the exact value > this approximate value\n", stdout);
            break;
    }

    snd_pcm_hw_params_get_period_time(hw_params, &approx_period_time, &dir);
    fprintf(stdout, "Approximate period time: %ld", (long)approx_period_time);
    switch (dir)
    {
        case (-1):
            fputs(", the exact value < this approximate value\n", stdout);
            break;
        case (0):
            fputs(", the exact value = this approximate value\n", stdout);
            break;
        case (1):
            fputs(", the exact value > this approximate value\n", stdout);
            break;
    }

    fprintf(stdout, "\n");
}

/****************************
 * Signal Handler Functions
 ****************************/

void toggle(int sig)
{
    signal_pause_switch = signal_pause_switch? 0:1;
}

/****************************
 * ALSA Related
 ****************************/

/**
 * make PCM device enter PREPARED state
 */
static int xrun_recovery(snd_pcm_t* handle, int err)
{
    int ret = 0;

    switch (err)
    {
        case (-EPIPE):
            fputs("WANR: Overrun! Try to prepare...\n", stderr);
            err = snd_pcm_prepare(handle);
            if (err < 0)
            {
                pr_error("Can't recover from over-run, prepare failed", err);
                err = -1;
            }
            break;
        case (-ESTRPIPE):
            fputs("WANR: Suspended! Try to prepare...\n", stderr);
            while ((err = snd_pcm_resume(handle)) != -EAGAIN)
            {
                fputs("\tTry again in 10ms\n", stderr);
                usleep(100000);
            }
            if (err < 0)
            {
                pr_error("Can't recover from over-run, prepare failed", err);
                err = -1;
            }
            break;
        case (-EBADFD):
            fputs("ERROR: Bad file discriptor, means ALSA handshake corrupts LOL\n", stderr);
            err = -1;
            break;
        default:
            pr_error("WARN: Something else status is in",  err);
            err = -1;
    }

    return err;
}

int prepare_device(const char* device_name, snd_pcm_t **handle)
{
    /* stream type */
    snd_pcm_stream_t stream     = SND_PCM_STREAM_CAPTURE;

    /* HW Params */
    snd_pcm_hw_params_t* hw_params;

    /* SW Params */
    snd_pcm_sw_params_t* sw_params;

    int err, dir;
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
    err = snd_pcm_hw_params_set_access(*handle, hw_params, access_type);
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
        fprintf(stderr, "WARN:HW:channel:\n\tExpect: %d\n\tApprox: %d\n", channel_expect, channel);
    }

    unsigned int rate_expect = rate;
    fprintf(stdout, "DIR is set to: %d\n", dir);
    err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &rate, &dir);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_hw_params_set_rate_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (rate != rate_expect)
    {
        fprintf(stderr, "WARN:HW:rate:\n\tExpect: %d\n\tApprox: %d\n",  rate_expect, rate);
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
        fprintf(stderr, "WARN:HW:period_time:\n\tExpect: %d\n\tApprox: %d\n", period_time_expect, period_time);
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
        fprintf(stderr, "WARN:HW:periods:\n\tExpect: %d\n\tApprox: %d\n", periods_expect, periods);
    }

    /*
    unsigned int buffer_time_expect = buffer_time;
    err = snd_pcm_hw_params_set_buffer_time_near(*handle, hw_params, &buffer_time, NULL);
    if (err < 0)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_buffer_time_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (buffer_time != buffer_time_expect)
    {
        fprintf(stderr, "WARN:HW:buffer_time:\n\tExpect: %d\n\tApprox: %d\n", buffer_time_expect, buffer_time);
    }
    */

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

    //dump_period_info(hw_params);

    /* Set SW Params */
    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) 
    {
        fprintf(stderr, "snd_pcm_sw_params_malloc failed: %s\n", snd_strerror(err));
        return 1;
    }

    err = snd_pcm_sw_params_current(*handle, sw_params);
    if (err < 0) {
            fprintf(stderr, "snd_pcm_params_current failed: %s\n", snd_strerror(err));
            return err;
    }

    // start threshold
    snd_pcm_uframes_t period_size;
    snd_pcm_hw_params_get_period_size(hw_params, &period_size, NULL);
    err = snd_pcm_sw_params_set_start_threshold(*handle, sw_params, start_threshold_factor*period_size);
    if (err < 0)
    {
        fprintf(stderr, "snd_pcm_sw_params_set_start_threshold failed: %s\n", snd_strerror(err));
        return 1;
    }
    
    // - set params
    err = snd_pcm_sw_params(*handle, sw_params);
    if (err < 0)
    {
        fprintf(stderr, "snd_pcm_sw_params failed: %s\n", snd_strerror(err));
        return 1;
    }
    else
    {
        fprintf(stdout, "Set SW parameters finished\n");
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
    // - sw info
    fprintf(stdout, "\n- SW Params -\n");
    snd_pcm_sw_params_dump(sw_params, snd_out);
    fprintf(stdout, "\n");

    snd_pcm_hw_params_free(hw_params);
    snd_pcm_sw_params_free(sw_params);

    return 0;
}

int main()
{
    const char* device_name = "hw:0,0";
    //const char* device_name = "sd_carplay_downlink_in";
    int ret;
    snd_pcm_t* handle;
    struct sigaction act;

    /* install signal handler */
    act.sa_handler = toggle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    ret = sigaction(SIGUSR1, &act, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "sigaction failed!\n");
        exit(1);
    }

    /* prepare PCM device */
    ret = prepare_device(device_name, &handle);
    if (ret != 0)
    {
        fprintf(stderr, "prepare_device failed!\n");
        exit(1);
    }

    /* start capture */
    char buf[100000];
    int loop = 0;
    snd_pcm_sframes_t cnt_avail_frame;
    snd_pcm_state_t pcm_state;
    const snd_pcm_channel_area_t* areas;
    snd_pcm_uframes_t offset, frames;
    const snd_pcm_uframes_t period_size = rate / 1000 * period_time / 1000;

    while (signal_pause_switch)
    {
#ifdef VERBOSE_LOG
        fprintf(stdout, "Loop: %d\n", loop++);
#endif
        // make sure we are always in RUNNING state
        pcm_state = snd_pcm_state(handle);
        switch (pcm_state)
        {
            case (SND_PCM_STATE_PREPARED):
                fputs("State transition: PREPARED -> RUNNING\n", stdout);
                ret = snd_pcm_start(handle);
                if (ret < 0)
                {
                    fputs("Failed!\n", stderr);
                    exit(1);
                }
                else
                    continue;

            case (SND_PCM_STATE_XRUN):
                fputs("State transition: XRUN -> RUNNING\n", stdout);
                if (xrun_recovery(handle, -EPIPE) < 0)
                    exit(1);
                else 
                    continue;

            case (SND_PCM_STATE_SUSPENDED):
                fputs("State transition: SUSPENDED -> RUNNING\n", stdout);
                if (xrun_recovery(handle, -ESTRPIPE) < 0)
                    exit(1);
                else 
                    continue;
            case (SND_PCM_STATE_RUNNING):
                break;
            default:
                fputs("Unknown entry state...\n", stderr);
                exit(1);
        }

        // get available frame
        cnt_avail_frame = snd_pcm_avail_update(handle);
        if (cnt_avail_frame < 0)
        {
            if (xrun_recovery(handle, cnt_avail_frame) < 0)
                exit(1);
            else
                continue;
        }
        else if (cnt_avail_frame < period_size)
        {
#ifdef VERBOSE_LOG
            fprintf(stdout, "available frame: %d\n", (int)cnt_avail_frame);
#endif
            fputs("Not enough available data, waiting...\n", stderr);
            // wait for PCM to be ready
            ret = snd_pcm_wait(handle, -1);
            if (ret < 0)
            {
                if (xrun_recovery(handle, ret) < 0)
                    exit(1);
                else 
                    continue;
            }
        }

#ifdef VERBOSE_LOG
        fputs("Enough data is in buffer\n", stdout);
#endif
        frames = period_size;  // this should equals to avail_min
        ret = snd_pcm_mmap_begin(handle, &areas, &offset, &frames);
        //ret = snd_pcm_readi(handle, buf, frames);
        if (ret < 0)
        {
            if (xrun_recovery(handle, ret) < 0)
                exit(1);
            else
                continue;
        }
        else
        {
            if (period_size != frames)
            {
                fprintf(stderr, "!!! Actual returned frames: %lu\n", frames);
            }
            dump_areainfo(areas);
            fprintf(stdout, "Offset: %lu(frame)\n", (unsigned long)offset);
            fprintf(stdout, "Frame: %lu(frame)\n", (unsigned long)frames);

            ret = snd_pcm_mmap_commit(handle, offset, frames);
            if (ret < 0 || ret != frames)
            {
                if (xrun_recovery(handle, ret >= 0 ? -EPIPE : ret) < 0)
                    exit(1);
                else
                    continue;
            }
        }

        /* Sleep so that we can get less log and get over-run */
        //sleep(1);
        usleep(10000); // 10ms
    }
}
