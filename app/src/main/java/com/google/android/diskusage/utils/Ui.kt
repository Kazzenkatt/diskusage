package com.google.android.diskusage.utils

import android.content.Context
import android.widget.Toast
import androidx.annotation.ColorInt
import androidx.annotation.StringRes
import androidx.core.content.ContextCompat
import android.util.TypedValue

object Ui {
    private val ctx: Context get() = AppHelper.appContext

    @JvmStatic
    fun toast(msg: String) = Toast.makeText(ctx, msg, Toast.LENGTH_SHORT).show()

    @JvmStatic
    fun toast(@StringRes resId: Int) = Toast.makeText(ctx, resId, Toast.LENGTH_SHORT).show()

    @JvmStatic
    fun longToast(msg: String) = Toast.makeText(ctx, msg, Toast.LENGTH_LONG).show()

    @JvmStatic
    fun appStr(@StringRes resId: Int, vararg args: Any): String = ctx.getString(resId, *args)

    @JvmStatic
    @ColorInt
    fun Context.styledColor(attr: Int): Int {
        val tv = TypedValue()
        theme.resolveAttribute(attr, tv, true)
        return tv.data
    }

    @JvmStatic
    fun Context.drawableCompat(resId: Int) = ContextCompat.getDrawable(this, resId)
}
