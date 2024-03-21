package org.telegram.sword.app.home.util

import org.telegram.messenger.MessagesController
import org.telegram.messenger.UserConfig
import org.telegram.sword.app.common.helper.ui.getTeleUserInId
import org.telegram.tgnet.ConnectionsManager
import org.telegram.tgnet.TLRPC
import org.telegram.ui.DialogsActivity.allContactList
import org.telegram.ui.DialogsActivity.onlineContactList
import java.util.*


var onlineContacts: ArrayList<TLRPC.User> = ArrayList()

var allContacts: ArrayList<TLRPC.TL_contact> = ArrayList()


fun updateContactsArray(contacts: ArrayList<TLRPC.TL_contact>) {

    try {

        if (onlineContactList.isEmpty() || allContactList.isEmpty()) {

            val currentAccount = UserConfig.selectedAccount

            val selfId = UserConfig.getInstance(currentAccount).clientUserId

            if (contacts.size > 1 && onlineContactList.size == 0) {

                (contacts as ArrayList<TLRPC.TL_contact>).forEachIndexed { index, contact ->

                    val user: TLRPC.User? =
                        getTeleUserInId((contact as TLRPC.TL_contact).user_id ?: -1)


                    if (user != null && user.id != selfId) {

                        if (user.status != null) {

                            val isOnline =
                                !user.bot && (user.status.expires > ConnectionsManager.getInstance(currentAccount).currentTime ||
                                        MessagesController.getInstance(currentAccount).onlinePrivacy.containsKey(user.id))
                            val isRecently =
                                !user.bot && (user.status.expires == -100)

                            if (isOnline) {

                                onlineContactList.add(0, user)

                            } else if (isRecently) {

                                onlineContactList.add(user)
                            }

                        }

                        allContactList.add(user)
                    }

                }

            }

        }


    } catch (e: Exception) {

    }

}

fun updateOnlineUsers():ArrayList<TLRPC.User>{

    onlineContactList.clear()

    val currentAccount = UserConfig.selectedAccount

    allContactList.forEach { user ->


        if (user.status !=null){

            val isOnline = !user.bot && (user.status.expires > ConnectionsManager.getInstance(currentAccount).currentTime ||
                        MessagesController.getInstance(currentAccount).onlinePrivacy.containsKey(user.id))

            val isRecently = !user.bot && (user.status.expires == -100)


                if (isOnline) {

                    onlineContactList.add(0, user)

                } else if (isRecently) {

                    onlineContactList.add(user)
                }


        }


    }

    return onlineContactList


}