package org.telegram.sword.app.common.extansion

import androidx.recyclerview.widget.RecyclerView

fun RecyclerView.pagingListener(  pagingFun:()->Unit) {

    this.addOnScrollListener(object : RecyclerView.OnScrollListener() {
        override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {}

        override fun onScrolled(recyclerView: RecyclerView, dx: Int, dy: Int) {

            if ((dy > 0 || dy < 0)) {

                pagingFun.invoke()
            }
        }
    })
}