package org.telegram.sword.app.common.helper.ui

import android.content.Context
import android.util.AttributeSet
import android.view.View
import android.widget.LinearLayout
import androidx.appcompat.widget.AppCompatEditText
import androidx.appcompat.widget.AppCompatImageView
import androidx.core.widget.addTextChangedListener
import androidx.lifecycle.LiveData
import org.telegram.messenger.R
import org.telegram.sword.app.common.AppConst.EMPTY
import org.telegram.sword.app.common.extansion.hide
import org.telegram.sword.app.common.extansion.show
import org.telegram.sword.app.common.helper.customize.SingleLiveEvent

class SwordSearchView: LinearLayout {


    private val _searchText = SingleLiveEvent<String>()

    val searchText: LiveData<String> get() = _searchText

    private var mRootContainer: View = inflate(context, R.layout.sword_search_view, this)

    var swordSearchView: AppCompatEditText = mRootContainer.findViewById(R.id.searchView)

    var clearSearch: AppCompatImageView = mRootContainer.findViewById(R.id.clearSearch)


    constructor(context: Context) : this(context, null)
    constructor(context: Context, attrs: AttributeSet?) : this(context, attrs, 0)

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(
    context,
    attrs,
    defStyleAttr
    ) {

        context.theme.obtainStyledAttributes(
            attrs,
            R.styleable.SwordSearchView, 0, 0
        ).apply {

            try {

                swordSearchView.addTextChangedListener {
                    _searchText.value = it.toString()
                    if (it.toString().isNotEmpty()){
                        clearSearch.show()
                    }else{
                        clearSearch.hide()
                    }
                }

                clearSearch.setOnClickListener{
                    swordSearchView.setText(EMPTY)
                }

                if (hasValue(R.styleable.SwordSearchView_search_hint)) {

                    swordSearchView.hint = getString(R.styleable.SwordSearchView_search_hint)
                }


            } finally {
                recycle()
            }
        }
    }

    fun getText():String =swordSearchView.text.toString()



}