
extern float GS_DEV;
extern float GS_Angle;
extern float detY0;
extern float detTheta0;

static float y_buffer[8];
static sliding_filter_t dety_filter = {8, 0, NULL};

void on_init(void)
{
    sliding_filter_init(&dety_filter, y_buffer);
}

void on_newtag(void)
{
    if(Motionstyle == MOTIONSTATE_GOSTRAIGHT)
    {
        UpdateAgvHeadDirToNew();
        Get_TwoDDev();

        if( (fabs(GS_DEV) < 30) && (fabs(GS_Angle) < 5) )
        {
          sliding_filter_input(&dety_filter, GS_DEV);
        }
    }
}


void on_action_over(void)
{
    float det;

    det = sliding_filter_output(&dety_filter);
    if(fabs(det) > 8)
    {
      detTheta0 += (det/2000);
      APP_TRACE("update detTheta0: %f\r\n", detTheta0);

      sliding_filter_init(&dety_filter, y_buffer);
    }
    else if(fabs(det) > 2)
    {
      detY0 += (det*0.3)/1000;
      APP_TRACE("update detY0: %f\r\n", detY0);

      sliding_filter_init(&dety_filter, y_buffer);
    }
}

