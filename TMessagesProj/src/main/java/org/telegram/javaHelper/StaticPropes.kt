package org.telegram.javaHelper


var staticProp:StaticsProp? = null

 fun getInitialStaticProp():StaticsProp{

    if (staticProp==null){

        staticProp = StaticsProp()
    }

    return staticProp!!
}