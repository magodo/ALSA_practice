/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 04 Aug 2016 09:08:09 AM CST
 File Name: my_capture.c
 Description: 
 ************************************************************************/

#include <alsa/asoundlib.h>
#include <signal.h>
#include <time.h>

#define VERBOSE_LOG

/****************************
 * Global Variables
 ****************************/

/* HW Param */
// - access type
static snd_pcm_access_t access_type                = SND_PCM_ACCESS_MMAP_INTERLEAVED;
// - stream param
static snd_pcm_format_t format                     = SND_PCM_FORMAT_S16_LE;
static unsigned int channel                        = 2;
static unsigned int rate                           = 44100;
// - buffer param
static unsigned int periods                        = 8;
static unsigned int period_time                    = 10000; // us == 100(ms)
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;

/* SW Param */
static unsigned int start_threshold_factor         = 0;

/* Capture toggle */
static volatile sig_atomic_t signal_pause_switch   = 1;

/****************************
 * Helper Functions
 ****************************/

void pr_error(const char* msg, int err)
{
    fflush(stdout);
    fprintf(stderr, "%s: %s\n", msg, snd_strerror(err));
}

void dump_areainfo(const snd_pcm_channel_area_t* areas)
{
    int chn;

    fprintf(stdout, "Area Info:\n");
    for (chn = 0; chn < channel; chn++)
    {
        fprintf(stdout, "Channe %d:", chn);
        fprintf(stdout, "Base address: %p\n", areas[chn].addr);
        fprintf(stdout, "Offset to first sample: %u(bit)\n", areas[chn].first);
        fprintf(stdout, "Sample distance: %u(bit)\n", areas[chn].step);
        fputs("\n", stdout);
    }
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
            fflush(stdout);
            fputs("WANR: XRun! Try to prepare...\n", stderr);
            err = snd_pcm_prepare(handle);
            if (err < 0)
            {
                pr_error("Can't recover from xrun, prepare failed", err);
                err = -1;
            }
            break;
        case (-ESTRPIPE):
            fflush(stdout);
            fputs("WANR: Suspended! Try to prepare...\n", stderr);
            while ((err = snd_pcm_resume(handle)) != -EAGAIN)
            {
                fflush(stdout);
                fputs("\tTry again in 10ms\n", stderr);
                usleep(100000);
            }
            if (err < 0)
            {
                pr_error("Can't recover from suspended, prepare failed", err);
                err = -1;
            }
            break;
        case (-EBADFD):
            fflush(stdout);
            fputs("ERROR: Bad file discriptor, means ALSA handshake corrupts LOL\n", stderr);
            err = -1;
            break;
        default:
            pr_error("WARN: Some else status is in",  err);
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

    int err;
    snd_pcm_uframes_t size;
    snd_output_t* snd_out;  // used for dumping PCM info

    /* 1. Open PCM device */
    err = snd_pcm_open(handle, device_name, stream, 0);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_open failed: %s\n", snd_strerror(err));
        return 1;
    }

    /* 2. Set HW Params */
    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_malloc failed: %s\n", snd_strerror(err));
        return 1;
    }

    // 2.1 init hw params
    err = snd_pcm_hw_params_any(*handle, hw_params);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_any failed: %s\n", snd_strerror(err));
        return 1;
    }

    // 2.2 access type
    err = snd_pcm_hw_params_test_access(*handle, hw_params, access_type);
    if (err < 0)
    {
        fflush(stdout);
        fprintf(stderr, "%s not supported\n", snd_pcm_access_name(access_type));
        return 1;
    }
    err = snd_pcm_hw_params_set_access(*handle, hw_params, access_type);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_set_access failed: %s\n", snd_strerror(err));
        return 1;
    }

    // 2.3 stream param

    // 2.3.1 format
    err = snd_pcm_hw_params_set_format(*handle, hw_params, format);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_set_format failed: %s\n", snd_strerror(err));
        return 1;
    }

    // 2.3.2 channel
    unsigned int channel_expect = channel;
    err = snd_pcm_hw_params_set_channels_near(*handle, hw_params, &channel);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_set_channels_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (channel != channel_expect)
    {
        fflush(stdout);
        fprintf(stderr, "WARN:HW:channel:\n\tExpect: %d\n\tApprox: %d\n", channel_expect, channel);
        return 1;
    }

    // 2.3.3 rate
    unsigned int rate_expect = rate;
    err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &rate, NULL);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_set_rate_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (rate != rate_expect)
    {
        fflush(stdout);
        fprintf(stderr, "WARN:HW:rate:\n\tExpect: %d\n\tApprox: %d\n",  rate_expect, rate);
        return 1;
    }

    // 2.4 buffer param

    // 2.4.1 period
    unsigned int period_time_expect = period_time;
    err = snd_pcm_hw_params_set_period_time_near(*handle, hw_params, &period_time, NULL);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_set_period_time_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (period_time != period_time_expect)
    {
        fflush(stdout);
        fprintf(stderr, "WARN:HW:period_time:\n\tExpect: %d\n\tApprox: %d\n", period_time_expect, period_time);
        return 1;
    }
    err = snd_pcm_hw_params_get_period_size(hw_params, &size, 0);
    if (err < 0)
    {
        pr_error("snd_pcm_hw_params_get_period_size failed", err);
        return 1;
    }
    period_size = size;

    // 2.4.2 buffer
    unsigned int periods_expect = periods;
    err = snd_pcm_hw_params_set_periods_near(*handle, hw_params, &periods, NULL);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_params_set_periods_near failed: %s\n", snd_strerror(err));
        return 1;
    }
    if (periods != periods_expect)
    {
        fflush(stdout);
        fprintf(stderr, "WARN:HW:periods:\n\tExpect: %d\n\tApprox: %d\n", periods_expect, periods);
        return 1;
    }
    err = snd_pcm_hw_params_get_buffer_size(hw_params, &size);
    if (err < 0)
    {
        pr_error("snd_pcm_hw_params_get_buffer_size failed", err);
        return 1;
    }
    buffer_size = size;

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

    // 2.5 set params
    err = snd_pcm_hw_params(*handle, hw_params);
    if (err < 0)
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_hw_paramsfailed: %s\n", snd_strerror(err));
        return 1;
    }
    else
    {
        fprintf(stdout, "Set HW parameters finished\n");
    }

    //dump_period_info(hw_params);

    /* 3. Set SW Params */
    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) 
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_sw_params_malloc failed: %s\n", snd_strerror(err));
        return 1;
    }

    // 3.1 init sw params
    err = snd_pcm_sw_params_current(*handle, sw_params);
    if (err < 0) {
            fflush(stdout);
            fprintf(stderr, "snd_pcm_params_current failed: %s\n", snd_strerror(err));
            return err;
    }

    // 3.2 start threshold
    err = snd_pcm_sw_params_set_start_threshold(*handle, sw_params, start_threshold_factor*period_size);
    if (err < 0)
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_sw_params_set_start_threshold failed: %s\n", snd_strerror(err));
        return 1;
    }
    
    // 3.3 set params
    err = snd_pcm_sw_params(*handle, sw_params);
    if (err < 0)
    {
        fflush(stdout);
        fprintf(stderr, "snd_pcm_sw_params failed: %s\n", snd_strerror(err));
        return 1;
    }
    else
    {
        fprintf(stdout, "Set SW parameters finished\n");
    }

    /* 4. Dump PCM information */
    err = snd_output_stdio_attach(&snd_out, stdout, 0);
    if (err < 0)
    {
        fflush(stdout);
        fprintf(stderr, "snd_output_stdio_attach failed: %s\n", snd_strerror(err));
        return 1;
    }
    // 4.1 hw info
    fprintf(stdout, "\n- HW Params -\n");
    snd_pcm_hw_params_dump(hw_params, snd_out);
    // 4.2 sw info
    fprintf(stdout, "\n- SW Params -\n");
    snd_pcm_sw_params_dump(sw_params, snd_out);
    fprintf(stdout, "\n");

    /* 5. Free memory */
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

    /* 0. install signal handler */
    act.sa_handler = toggle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    ret = sigaction(SIGUSR1, &act, NULL);
    if (ret != 0)
    {
        fflush(stdout);
        fprintf(stderr, "sigaction failed!\n");
        exit(1);
    }

    /* prepare PCM device */
    ret = prepare_device(device_name, &handle);
    if (ret != 0)
    {
        fflush(stdout);
        fprintf(stderr, "prepare_device failed!\n");
        exit(1);
    }


    /* start capture */
    //char buf[100000];
    int loop = 0;
    snd_pcm_sframes_t cnt_avail_frame;
    const snd_pcm_channel_area_t* areas;
    snd_pcm_state_t pcm_state;
    snd_pcm_uframes_t offset, frames;
    struct timespec tp_start, tp_end;

    while (signal_pause_switch)
    {
#ifdef VERBOSE_LOG
        fprintf(stdout, "********************\nLoop: %d\n", loop++);
#endif
        // make sure we are always in RUNNING state
        pcm_state = snd_pcm_state(handle);
        switch (pcm_state)
        {
            case (SND_PCM_STATE_PREPARED):
                fputs("State transition: PREPARED -> RUNNING\n", stdout);
                ret = snd_pcm_start(handle);
                //fprintf(stdout, "Current state: %s\n", snd_pcm_state_name(snd_pcm_state(handle)));
                if (ret < 0)
                {
                    fflush(stdout);
                    fputs("Failed!\n", stderr);
                    exit(1);
                }
                else
                    continue;

            case (SND_PCM_STATE_XRUN):
                fputs("State transition: XRUN -> PREPARED\n", stdout);
                if (xrun_recovery(handle, -EPIPE) < 0)
                    exit(1);
                else 
                    continue;

            case (SND_PCM_STATE_SUSPENDED):
                fputs("State transition: SUSPENDED -> PREPARED\n", stdout);
                if (xrun_recovery(handle, -ESTRPIPE) < 0)
                    exit(1);
                else 
                    continue;
            case (SND_PCM_STATE_RUNNING):
                break;
            default:
                fflush(stdout);
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
        else
        {
#ifdef VERBOSE_LOG
            fprintf(stdout, "Available frame in ring buffer: %d\n", (int)cnt_avail_frame);
#endif
            if (cnt_avail_frame < period_size)
            {
                fflush(stdout);
                fputs("Not enough available data, waiting...\n", stderr);
                // wait for PCM to be ready(i.e. available frame >= avail_min (which equals to "period_size" by default))
#ifdef VERBOSE_LOG
                clock_gettime(CLOCK_REALTIME, &tp_start);
#endif
                ret = snd_pcm_wait(handle, -1);
                if (ret < 0)
                {
                    if (xrun_recovery(handle, ret) < 0)
                        exit(1);
                    else 
                        continue;
                }
#ifdef VERBOSE_LOG
                // get available frame
                cnt_avail_frame = snd_pcm_avail_update(handle);
                clock_gettime(CLOCK_REALTIME, &tp_end);
                fprintf(stdout, "Enough data is in buffer: %lu (duration: %ld ms)\n",
                        cnt_avail_frame,
                        1000*(tp_end.tv_sec-tp_start.tv_sec)+(tp_end.tv_nsec-tp_start.tv_nsec)/1000000);
#endif
            }
        }

        frames = period_size;
        ret = snd_pcm_mmap_begin(handle, &areas, &offset, &frames);  // we want to read one period_size frames
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
                fflush(stdout);
                fprintf(stderr, "!!! Actual available frames: %lu; expected: %lu\n", frames, period_size);
            }
            dump_areainfo(areas);
            fprintf(stdout, "Offset: %lu(frame)\n", (unsigned long)offset);
            fprintf(stdout, "Frame: %lu(frame)\n", (unsigned long)frames);

            ret = snd_pcm_mmap_commit(handle, offset, frames);   // one period frames read
            if (ret < 0 || ret != frames)
            {
                if (xrun_recovery(handle, ret >= 0 ? -EPIPE : ret) < 0)
                    exit(1);
                else
                    continue;
            }
        }

        /* Sleep so that we can get less log and get over-run */
        usleep(11000); // 11ms

        fprintf(stdout, "********************\n");
    }
}
