package de.pleumann.railuinoapp;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.support.v4.view.ViewPager;
import android.support.v4.view.ViewPager.OnPageChangeListener;
import android.util.AttributeSet;
import android.view.View;

public class PageIndicator extends View implements OnPageChangeListener {

    private int count;
    
    private int index;
    
    private float step;
    
    private ViewPager pager;
    
    public PageIndicator(Context context, AttributeSet attrs) {
        super(context, attrs);
    }
    
    public PageIndicator(Context context) {
        super(context);
    }

    public void setPager(ViewPager pager) {
        this.pager = pager;
        onPageSelected(pager.getCurrentItem());
    }
    
    @Override
    public void onPageScrolled(int arg0, float arg1, int arg2) {
    }
    
    @Override
    public void onPageScrollStateChanged(int arg0) {
    }
    
    @Override
    public void onPageSelected(int arg0) {
        count = pager.getAdapter().getCount();
        index = arg0;
        
        step = (float)(getWidth() - getPaddingLeft() - getPaddingRight()) / (float)(count);

        invalidate();
    }
    
    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        count = pager.getAdapter().getCount();
        index = pager.getCurrentItem();
        
        step = (float)(getWidth() - getPaddingLeft() - getPaddingRight()) / (float)(count);

        Paint paint = new Paint();
        
        paint.setColor(Color.LTGRAY);
        canvas.drawRect(getPaddingLeft(), getPaddingBottom(), getWidth() - getPaddingRight(), getHeight() - getPaddingTop(), paint);
        paint.setColor(Color.CYAN);
        canvas.drawRect(getPaddingLeft() + index * step, getPaddingBottom(), getPaddingLeft() + (index + 1) * step, getHeight() - getPaddingTop(), paint);
    }
    
}
