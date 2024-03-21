package org.telegram.ui.ActionBar;

import static org.telegram.ui.ActionBar.Theme.*;

import android.graphics.Color;
import android.util.SparseArray;

import androidx.core.graphics.ColorUtils;

import java.util.HashMap;

public class ThemeColors {

    private static SparseArray<String> colorKeysMap;
    private static HashMap<String, Integer> colorKeysStringMap;
    public static int[] createDefaultColors() {
        int[] defaultColors = new int[Theme.colorsCount];

        defaultColors[key_wallpaperFileOffset] = 0;
        defaultColors[key_dialogBackground] = 0xffffffff;
        defaultColors[key_dialogBackgroundGray] = 0xfff0f0f0;
        defaultColors[key_dialogTextBlack] = 0xff222222;
        defaultColors[key_dialogTextLink] =0xff093489;//appBlue
        defaultColors[key_dialogLinkSelection] = 0x3362a9e3;
        defaultColors[key_dialogTextBlue] =  0xff093489;//appBlue
        defaultColors[key_dialogTextBlue2] = 0xff093489;//appBlue
        defaultColors[key_dialogTextBlue4] = 0xff093489;//appBlue
        defaultColors[key_dialogTextGray] =0xff093489;//appBlue
        defaultColors[key_dialogTextGray2] = 0xff757575;
        defaultColors[key_dialogTextGray3] = 0xff999999;
        defaultColors[key_dialogTextGray4] = 0xffb3b3b3;
        defaultColors[key_dialogTextHint] = 0xff979797;
        defaultColors[key_dialogIcon] = 0xff676b70;
        defaultColors[key_dialogGrayLine] = 0xffd2d2d2;
        defaultColors[key_dialogTopBackground] = 0xff093489;//appBlue
        defaultColors[key_dialogInputField] = 0xffdbdbdb;
        defaultColors[key_dialogInputFieldActivated] = 0xff093489;//appBlue
        defaultColors[key_dialogCheckboxSquareBackground] = 0xff093489;//appBlue
        defaultColors[key_dialogCheckboxSquareCheck] = 0xffffffff;
        defaultColors[key_dialogCheckboxSquareUnchecked] = 0xff737373;
        defaultColors[key_dialogCheckboxSquareDisabled] = 0xffb0b0b0;
        defaultColors[key_dialogRadioBackground] = 0xffb3b3b3;
        defaultColors[key_dialogRadioBackgroundChecked] = 0xff093489;//appBlue
        defaultColors[key_dialogLineProgress] = 0xff527da3;
        defaultColors[key_dialogLineProgressBackground] = 0xffdbdbdb;
        defaultColors[key_dialogButton] = 0xff093489;//appBlue
        defaultColors[key_dialogButtonSelector] = 0xff093489;//appBlue
        defaultColors[key_dialogScrollGlow] = 0xfff5f6f7;
        defaultColors[key_dialogRoundCheckBox] = 0xff4cb4f5;//1232456
        defaultColors[key_dialogRoundCheckBoxCheck] = 0xffffffff;
        defaultColors[key_dialogCameraIcon] = 0xffffffff;
        defaultColors[key_dialog_inlineProgressBackground] = 0xf6f0f2f5;
        defaultColors[key_dialog_inlineProgress] = 0xff6b7378;
        defaultColors[key_dialogSearchBackground] = 0xfff2f4f5;
        defaultColors[key_dialogSearchHint] = 0xff98a0a7;
        defaultColors[key_dialogSearchIcon] = 0xffa1a8af;
        defaultColors[key_dialogSearchText] = 0xff222222;
        defaultColors[key_dialogFloatingButton] = 0xff093489;//appBlue
        defaultColors[key_dialogFloatingButtonPressed] = 0x0f000000;
        defaultColors[key_dialogFloatingIcon] = 0xffffffff;
        defaultColors[key_dialogShadowLine] = 0x12000000;
        defaultColors[key_dialogEmptyImage] = 0xff9fa4a8;
        defaultColors[key_dialogEmptyText] = 0xff8c9094;
        defaultColors[key_dialogSwipeRemove] = 0xffe56555;
        defaultColors[key_dialogReactionMentionBackground] = 0xffF05459;

        defaultColors[key_windowBackgroundWhite] = 0xffffffff;
        defaultColors[key_windowBackgroundUnchecked] = 0xff9da7b1;
        defaultColors[key_windowBackgroundChecked] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundCheckText] = 0xffffffff;
        defaultColors[key_progressCircle] = 0xff1c93e3;
        defaultColors[key_windowBackgroundWhiteGrayIcon] = 0xff81868b;
        defaultColors[key_windowBackgroundWhiteBlueText] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueText2] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueText3] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueText4] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueText5] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueText6] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueText7] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueButton] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteBlueIcon] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteGreenText] = 0xff26972c;
        defaultColors[key_windowBackgroundWhiteGreenText2] = 0xff37a818;
        defaultColors[key_text_RedRegular] = 0xffcc2929;
        defaultColors[key_text_RedBold] = 0xffcc4747;
        defaultColors[key_fill_RedNormal] = 0xffeb5e5e;
        defaultColors[key_windowBackgroundWhiteGrayText] = 0xff838c96;
        defaultColors[key_windowBackgroundWhiteGrayText2] = 0xff82868a;
        defaultColors[key_windowBackgroundWhiteGrayText3] = 0xff999999;
        defaultColors[key_windowBackgroundWhiteGrayText4] = 0xff808080;
        defaultColors[key_windowBackgroundWhiteGrayText5] = 0xffa3a3a3;
        defaultColors[key_windowBackgroundWhiteGrayText6] = 0xff757575;
        defaultColors[key_windowBackgroundWhiteGrayText7] = 0xffc6c6c6;
        defaultColors[key_windowBackgroundWhiteGrayText8] = 0xff6d6d72;
        defaultColors[key_windowBackgroundWhiteBlackText] = 0xff222222;
        defaultColors[key_windowBackgroundWhiteHintText] = 0xffa8a8a8;
        defaultColors[key_windowBackgroundWhiteValueText] =0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteLinkText] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteLinkSelection] = 0x3362a9e3;
        defaultColors[key_windowBackgroundWhiteBlueHeader] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundWhiteInputField] = 0xffdbdbdb;
        defaultColors[key_windowBackgroundWhiteInputFieldActivated] = 0xff093489;//appBlue
        defaultColors[key_switchTrack] = 0xffb0b5ba;
        defaultColors[key_switchTrackChecked] = 0xff093489;//appBlue
        defaultColors[key_switchTrackBlue] = 0xff828e99;
        defaultColors[key_switchTrackBlueChecked] = 0xff093489;//appBlue
        defaultColors[key_switchTrackBlueThumb] = 0xffffffff;
        defaultColors[key_switchTrackBlueThumbChecked] = 0xffffffff;
        defaultColors[key_switchTrackBlueSelector] = 0x17404a53;
        defaultColors[key_switchTrackBlueSelectorChecked] = 0x21024781;
        defaultColors[key_switch2Track] = 0xfff57e7e;
        defaultColors[key_switch2TrackChecked] = 0xff093489;//appBlue
        defaultColors[key_checkboxSquareBackground] = 0xff093489;//appBlue
        defaultColors[key_checkboxSquareCheck] = 0xffffffff;
        defaultColors[key_checkboxSquareUnchecked] = 0xff737373;
        defaultColors[key_checkboxSquareDisabled] = 0xffb0b0b0;
        defaultColors[key_listSelector] = 0x0f000000;
        defaultColors[key_radioBackground] = 0xffb3b3b3;
        defaultColors[key_radioBackgroundChecked] = 0xff093489;//appBlue
        defaultColors[key_windowBackgroundGray] = 0xfff0f0f0;
        defaultColors[key_windowBackgroundGrayShadow] = 0xff000000;
        defaultColors[key_emptyListPlaceholder] = 0xff959595;
        defaultColors[key_divider] = 0xffd9d9d9;
        defaultColors[key_graySection] = 0xfff5f5f5;
        defaultColors[key_graySectionText] = 0xff82878A;
        defaultColors[key_contextProgressInner1] = 0xffbfdff6;
        defaultColors[key_contextProgressOuter1] = 0xff093489;//appBlue
        defaultColors[key_contextProgressInner2] = 0xffbfdff6;
        defaultColors[key_contextProgressOuter2] = 0xffffffff;
        defaultColors[key_contextProgressInner3] = 0xffb3b3b3;
        defaultColors[key_contextProgressOuter3] = 0xffffffff;
        defaultColors[key_contextProgressInner4] = 0xffcacdd0;
        defaultColors[key_contextProgressOuter4] = 0xff2f3438;
        defaultColors[key_fastScrollActive] = 0xff093489;//appBlue
        defaultColors[key_fastScrollInactive] = 0xffc9cdd1;
        defaultColors[key_fastScrollText] = 0xffffffff;

        defaultColors[key_avatar_text] = 0xffffffff;

        defaultColors[key_avatar_backgroundSaved] = 0xff093489;//appBlue
        defaultColors[key_avatar_background2Saved] = 0xff3D9DE0;
        defaultColors[key_avatar_backgroundArchived] = 0xffB8C2CC;
        defaultColors[key_avatar_backgroundArchivedHidden] = 0xff66bffa;
        defaultColors[key_avatar_backgroundRed] = 0xffFF845E;
        defaultColors[key_avatar_backgroundOrange] = 0xffFEBB5B;
        defaultColors[key_avatar_backgroundViolet] = 0xffB694F9;
        defaultColors[key_avatar_backgroundGreen] = 0xff9AD164;
        defaultColors[key_avatar_backgroundCyan] = 0xff093489;//appBlue
        defaultColors[key_avatar_backgroundBlue] = 0xff5CAFFA;
        defaultColors[key_avatar_backgroundPink] = 0xffFF8AAC;

        defaultColors[key_avatar_background2Red] = 0xffD45246;
        defaultColors[key_avatar_background2Orange] = 0xffF68136;
        defaultColors[key_avatar_background2Violet] = 0xff6C61DF;
        defaultColors[key_avatar_background2Green] = 0xff46BA43;
        defaultColors[key_avatar_background2Cyan] = 0xff359AD4;
        defaultColors[key_avatar_background2Blue] = 0xff408ACF;
        defaultColors[key_avatar_background2Pink] = 0xffD95574;

        defaultColors[key_avatar_backgroundInProfileBlue] = 0xff093489;//appBlue
        defaultColors[key_avatar_backgroundActionBarBlue] = 0xff093489;//appBlue
        defaultColors[key_avatar_subtitleInProfileBlue] = 0xffd7eafa;
        defaultColors[key_avatar_actionBarSelectorBlue] = 0xff093489;//appBlue
        defaultColors[key_avatar_actionBarIconBlue] = 0xffffffff;

        defaultColors[key_avatar_nameInMessageRed] = 0xffca5650;
        defaultColors[key_avatar_nameInMessageOrange] = 0xffd87b29;
        defaultColors[key_avatar_nameInMessageViolet] =  0xff093489;//appBlue
        defaultColors[key_avatar_nameInMessageGreen] = 0xff50b232;
        defaultColors[key_avatar_nameInMessageCyan] = 0xff093489;//appBlue
        defaultColors[key_avatar_nameInMessageBlue] = 0xff093489;//appBlue
        defaultColors[key_avatar_nameInMessagePink] = 0xff093489;//appBlue

        defaultColors[key_actionBarDefault] = Color.WHITE;//123456
//        defaultColors[key_actionBarDefaultIcon] = 0xff676a6f;
        defaultColors[key_actionBarDefaultIcon] = 0xff093489;//appBlue
        defaultColors[key_actionBarActionModeDefault] = 0xffffffff;
        defaultColors[key_actionBarActionModeDefaultTop] = 0x10000000;
//        defaultColors[key_actionBarActionModeDefaultIcon] = 0xff676a6f;
        defaultColors[key_actionBarActionModeDefaultIcon] = 0xff093489;//appBlue
        defaultColors[key_actionBarDefaultTitle] = 0xff676a6f;
        defaultColors[key_actionBarDefaultSubtitle] = 0xff676a6f;
        defaultColors[key_actionBarDefaultSelector] = 0xff093489;//appBlue
        defaultColors[key_actionBarWhiteSelector] = 0x1d000000;
        defaultColors[key_actionBarDefaultSearch] = 0xffffffff;
        defaultColors[key_actionBarDefaultSearchPlaceholder] = 0x88ffffff;
        defaultColors[key_actionBarDefaultSubmenuItem] = 0xff222222;
        defaultColors[key_actionBarDefaultSubmenuItemIcon] = 0xff676b70;
        defaultColors[key_actionBarDefaultSubmenuBackground] = 0xffffffff;
        defaultColors[key_actionBarDefaultSubmenuSeparator] = 0xfff5f5f5;
        defaultColors[key_actionBarActionModeDefaultSelector] = 0xffe2e2e2;
        defaultColors[key_actionBarTabActiveText] = 0xffffffff;
        defaultColors[key_actionBarTabUnactiveText] = 0xffd5e8f7;
        defaultColors[key_actionBarTabLine] = 0xffffffff;
        defaultColors[key_actionBarTabSelector] = 0xff093489;//appBlue

        defaultColors[key_actionBarBrowser] = 0xffffffff;

        defaultColors[key_actionBarDefaultArchived] = 0xff6f7a87;
        defaultColors[key_actionBarDefaultArchivedSelector] = 0xff5e6772;
        defaultColors[key_actionBarDefaultArchivedIcon] = 0xffffffff;
        defaultColors[key_actionBarDefaultArchivedTitle] = 0xffffffff;
        defaultColors[key_actionBarDefaultArchivedSearch] = 0xffffffff;
        defaultColors[key_actionBarDefaultArchivedSearchPlaceholder] = 0x88ffffff;

        defaultColors[key_chats_onlineCircle] = 0xff4bcb1c;
//        defaultColors[key_chats_unreadCounter] = 0xff4ecc5e;
        defaultColors[key_chats_unreadCounter] = 0xff093489;//appBlue
        defaultColors[key_chats_unreadCounterMuted] = 0xffc6c9cc;
        defaultColors[key_chats_unreadCounterText] = 0xffffffff;
        defaultColors[key_chats_archiveBackground] = 0xff093489;//appBlue
        defaultColors[key_chats_archivePinBackground] = 0xff9faab3;
        defaultColors[key_chats_archiveIcon] = 0xffffffff;
        defaultColors[key_chats_archiveText] = 0xffffffff;
        defaultColors[key_chats_name] = 0xff222222;
        defaultColors[key_chats_nameArchived] = 0xff525252;
        defaultColors[key_chats_secretName] = 0xff00a60e;
        defaultColors[key_chats_secretIcon] = 0xff19b126;
        defaultColors[key_chats_pinnedIcon] = 0xffa8a8a8;
        defaultColors[key_chats_message] = 0xff8b8d8f;
        defaultColors[key_chats_messageArchived] = 0xff919191;
        defaultColors[key_chats_message_threeLines] = 0xff8e9091;
        defaultColors[key_chats_draft] = 0xffdd4b39;
        defaultColors[key_chats_nameMessage] = 0xff093489;//appBlue
        defaultColors[key_chats_nameMessageArchived] = 0xff8b8d8f;
        defaultColors[key_chats_nameMessage_threeLines] = 0xff424449;
        defaultColors[key_chats_nameMessageArchived_threeLines] = 0xff5e5e5e;
        defaultColors[key_chats_attachMessage] = 0xff093489;//appBlue
        defaultColors[key_chats_actionMessage] = 0xff093489;//appBlue
        defaultColors[key_chats_date] = 0xff95999C;
        defaultColors[key_chats_pinnedOverlay] = 0x08000000;
        defaultColors[key_chats_tabletSelectedOverlay] = 0x0f000000;
//        defaultColors[key_chats_sentCheck] = 0xff46aa36;
        defaultColors[key_chats_sentCheck] = 0xff093489;//appBlue
//        defaultColors[key_chats_sentReadCheck] = 0xff46aa36;
        defaultColors[key_chats_sentReadCheck] = 0xff093489;//appBlue
//        defaultColors[key_chats_sentClock] = 0xff75bd5e;
        defaultColors[key_chats_sentClock] =  0xff093489;//appBlue
        defaultColors[key_chats_sentError] = 0xffd55252;
        defaultColors[key_chats_sentErrorIcon] = 0xffffffff;
        defaultColors[key_chats_verifiedBackground] = 0xff093489;//appBlue
        defaultColors[key_chats_verifiedCheck] = 0xffffffff;
        defaultColors[key_chats_muteIcon] = 0xffbdc1c4;
        defaultColors[key_chats_mentionIcon] = 0xffffffff;
        defaultColors[key_chats_menuBackground] = 0xffffffff;
        defaultColors[key_chats_menuItemText] = 0xff444444;
        defaultColors[key_chats_menuItemCheck] = 0xff093489;//appBlue
        defaultColors[key_chats_menuItemIcon] = 0xff889198;
        defaultColors[key_chats_menuName] = 0xffffffff;
        defaultColors[key_chats_menuPhone] = 0xffffffff;
        defaultColors[key_chats_menuPhoneCats] = 0xffc2e5ff;
        defaultColors[key_chats_actionIcon] = 0xffffffff;
        defaultColors[key_chats_actionBackground] = 0xff093489;//appBlue
        defaultColors[key_chats_actionPressedBackground] = 0xff093489;//appBlue
        defaultColors[key_chats_menuTopBackgroundCats] = 0xff598fba;
        defaultColors[key_chats_archivePullDownBackground] = 0xffc6c9cc;
        defaultColors[key_chats_archivePullDownBackgroundActive] = 0xff66a9e0;

        defaultColors[key_chat_attachCheckBoxCheck] = 0xffffffff;
        defaultColors[key_chat_attachCheckBoxBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_attachPhotoBackground] = 0x0c000000;
        defaultColors[key_chat_attachActiveTab] = 0xff093489;//appBlue
        defaultColors[key_chat_attachUnactiveTab] = 0xff92999e;
        defaultColors[key_chat_attachPermissionImage] = 0xff333333;
        defaultColors[key_chat_attachPermissionMark] = 0xffe25050;
        defaultColors[key_chat_attachPermissionText] = 0xff6f777a;
        defaultColors[key_chat_attachEmptyImage] = 0xffcccccc;

        defaultColors[key_chat_attachIcon] = 0xffffffff;
        defaultColors[key_chat_attachGalleryBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_attachGalleryText] = 0xff2e8de9;
        defaultColors[key_chat_attachAudioBackground] = 0xffeb6060;
        defaultColors[key_chat_attachAudioText] = 0xffde4747;
        defaultColors[key_chat_attachFileBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_attachFileText] = 0xff14a8e4;
        defaultColors[key_chat_attachContactBackground] = 0xfff2c04b;
        defaultColors[key_chat_attachContactText] = 0xffdfa000;
        defaultColors[key_chat_attachLocationBackground] = 0xff60c255;
        defaultColors[key_chat_attachLocationText] = 0xff3cab2f;
        defaultColors[key_chat_attachPollBackground] = 0xfff2c04b;
        defaultColors[key_chat_attachPollText] = 0xffdfa000;

        defaultColors[key_chat_inPollCorrectAnswer] = 0xff60c255;
        defaultColors[key_chat_outPollCorrectAnswer] = 0xff60c255;
        defaultColors[key_chat_inPollWrongAnswer] = 0xffeb6060;
        defaultColors[key_chat_outPollWrongAnswer] = 0xffeb6060;

        defaultColors[key_chat_status] =0xff093489;//appBlue
        defaultColors[key_chat_inGreenCall] = 0xff00c853;
        defaultColors[key_chat_outGreenCall] = 0xff00c853;
        defaultColors[key_chat_lockIcon] = 0xffffffff;
        defaultColors[key_chat_muteIcon] = 0xffb1cce3;
//        defaultColors[key_chat_inBubble] = 0xffffffff;
        defaultColors[key_chat_inBubble] = 0xFFFAFAFA; //TODO User MESSAGE COLOR
//        defaultColors[key_chat_inBubbleSelected] = 0xffecf7fd;
        defaultColors[key_chat_inBubbleSelected]  = 0xFFFAFAFA;  //TODO User MESSAGE SELECTED COLOR
        defaultColors[key_chat_inBubbleShadow] = 0xff1d3753;
//        defaultColors[key_chat_outBubble] = 0xffefffde;


        defaultColors[key_chat_myFSwors] = 0xff093489;

        defaultColors[key_chat_outBubble] = 0xFFACCDFF;  //TODO MY MESSAGE COLOR

        defaultColors[key_chat_outBubbleGradientSelectedOverlay] = 0x14000000;
//        defaultColors[key_chat_outBubbleSelected] = 0xffd9f7c5;
        defaultColors[key_chat_outBubbleSelected] = 0xFFACCDFF;//TODO MY MESSAGE SELECTED COLOR
        defaultColors[key_chat_outBubbleShadow] = 0xff1e750c;
        defaultColors[key_chat_inMediaIcon] = 0xffffffff;
        defaultColors[key_chat_inMediaIconSelected] = 0xffeff8fe;
//        defaultColors[key_chat_outMediaIcon] = 0xffefffde;
        defaultColors[key_chat_outMediaIcon] = 0xFFACCDFF;  // out_voice_play_icon
//        defaultColors[key_chat_outMediaIconSelected] = 0xffe1f8cf;
        defaultColors[key_chat_outMediaIconSelected] = 0xFFACCDFF;  // out_voice_play_icon
//        defaultColors[key_chat_messageTextIn] = 0xff000000;
        defaultColors[key_chat_messageTextIn] = 0xff2F3032;
//        defaultColors[key_chat_messageTextOut] = 0xff000000;
        defaultColors[key_chat_messageTextOut] = 0xff2F3032;
        defaultColors[key_chat_messageLinkIn] = 0xff093489;//appBlue
        defaultColors[key_chat_messageLinkOut] = 0xff093489;//appBlue
        defaultColors[key_chat_serviceText] = 0xffffffff;
        defaultColors[key_chat_serviceLink] = 0xffffffff;
        defaultColors[key_chat_serviceIcon] = 0xffffffff;
        defaultColors[key_chat_mediaTimeBackground] = 0x66000000;
//        defaultColors[key_chat_outSentCheck] = 0xff5db050;
        defaultColors[key_chat_outSentCheck] =  0xff51D323;// kanach /appBlue  1
//        defaultColors[key_chat_outSentCheckSelected] = 0xff5db050;
        defaultColors[key_chat_outSentCheckSelected] =  0xff51D323;// kanach /appBlue  2
//        defaultColors[key_chat_outSentCheckRead] = 0xff5db050;
        defaultColors[key_chat_outSentCheckRead] = 0xff51D323;// kanach /appBlue  3
//        defaultColors[key_chat_outSentCheckReadSelected] = 0xff5db050;
        defaultColors[key_chat_outSentCheckReadSelected] =  0xff51D323;// kanach /appBlue  4
//        defaultColors[key_chat_outSentClock] = 0xff75bd5e;
        defaultColors[key_chat_outSentClock] =  0xff093489;//appBlue
//        defaultColors[key_chat_outSentClockSelected] = 0xff75bd5e;
        defaultColors[key_chat_outSentClockSelected] = 0xff093489;//appBlue
//        defaultColors[key_chat_inSentClock] = 0xffa1aab3;
        defaultColors[key_chat_inSentClock] =  0xff093489;//appBlue
//        defaultColors[key_chat_inSentClockSelected] = 0xff93bdca;
        defaultColors[key_chat_inSentClockSelected] = 0xff093489;//appBlue
        defaultColors[key_chat_mediaSentCheck] = 0xffffffff;
//        defaultColors[key_chat_mediaSentCheck] = 0xff3497F9;
        defaultColors[key_chat_mediaSentClock] = 0xffffffff;
//        defaultColors[key_chat_mediaSentClock] = 0xff3497F9;
        defaultColors[key_chat_inViews] = 0xffa1aab3;
        defaultColors[key_chat_inViewsSelected] = 0xff93bdca;
        defaultColors[key_chat_outViews] = 0xff6eb257;
        defaultColors[key_chat_outViewsSelected] = 0xff6eb257;
        defaultColors[key_chat_mediaViews] = 0xffffffff;
        defaultColors[key_chat_inMenu] = 0xffb6bdc5;
        defaultColors[key_chat_inMenuSelected] = 0xff98c1ce;
//        defaultColors[key_chat_outMenu] = 0xff91ce7e;
        defaultColors[key_chat_outMenu] = 0xffffffff;
//        defaultColors[key_chat_outMenuSelected] = 0xff91ce7e;
        defaultColors[key_chat_outMenuSelected] = 0xffffffff;
        defaultColors[key_chat_mediaMenu] = 0xffffffff;
//        defaultColors[key_chat_outInstant] = 0xff55ab4f;
        defaultColors[key_chat_outInstant] =0xffffffff;
        defaultColors[key_chat_outInstantSelected] = 0xff489943;
        defaultColors[key_chat_inInstant] = 0xff093489;//appBlue
        defaultColors[key_chat_inInstantSelected] = 0xff093489;//appBlue
        defaultColors[key_chat_sentError] = 0xffdb3535;
        defaultColors[key_chat_sentErrorIcon] = 0xffffffff;
        defaultColors[key_chat_selectedBackground] = 0x280a90f0;
        defaultColors[key_chat_previewDurationText] = 0xffffffff;
        defaultColors[key_chat_previewGameText] = 0xffffffff;
        defaultColors[key_chat_inPreviewInstantText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outPreviewInstantText] = 0xff55ab4f;
        defaultColors[key_chat_outPreviewInstantText] = 0xffffffff;
        defaultColors[key_chat_secretTimeText] = 0xffe4e2e0;
        defaultColors[key_chat_stickerNameText] = 0xffffffff;
        defaultColors[key_chat_botButtonText] = 0xffffffff;
        defaultColors[key_chat_inForwardedNameText] = 0xff093489;//appBlue
        defaultColors[key_chat_outForwardedNameText] = 0xffffffff;
//        defaultColors[key_chat_outForwardedNameText] = 0xff55ab4f;
        defaultColors[key_chat_inPsaNameText] = 0xff5a9c39;
        defaultColors[key_chat_outPsaNameText] = 0xff5a9c39;
        defaultColors[key_chat_inViaBotNameText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outViaBotNameText] = 0xff55ab4f;
        defaultColors[key_chat_outViaBotNameText] = 0xffffffff;
        defaultColors[key_chat_stickerViaBotNameText] = 0xffffffff;
        defaultColors[key_chat_inReplyLine] = 0xff093489;//appBlue
//        defaultColors[key_chat_outReplyLine] = 0xff6eb969;
        defaultColors[key_chat_outReplyLine] = 0xffffffff;
        defaultColors[key_chat_stickerReplyLine] = 0xffffffff;
        defaultColors[key_chat_inReplyNameText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outReplyNameText] = 0xff55ab4f;
        defaultColors[key_chat_outReplyNameText] = 0xffffffff;
        defaultColors[key_chat_stickerReplyNameText] = 0xffffffff;
        defaultColors[key_chat_inReplyMessageText] = 0xff000000;
        defaultColors[key_chat_outReplyMessageText] = 0xff000000;
        defaultColors[key_chat_inReplyMediaMessageText] = 0xffa1aab3;
//        defaultColors[key_chat_outReplyMediaMessageText] = 0xff65b05b;
        defaultColors[key_chat_outReplyMediaMessageText] = 0xffa1aab3;
        defaultColors[key_chat_inReplyMediaMessageSelectedText] = 0xff89b4c1;
//        defaultColors[key_chat_outReplyMediaMessageSelectedText] = 0xff65b05b;
        defaultColors[key_chat_outReplyMediaMessageSelectedText] = 0xffa1aab3;
        defaultColors[key_chat_stickerReplyMessageText] = 0xffffffff;
        defaultColors[key_chat_inPreviewLine] = 0xff093489;//appBlue
//        defaultColors[key_chat_outPreviewLine] = 0xff88c97b;
        defaultColors[key_chat_outPreviewLine] = 0xffffffff;
        defaultColors[key_chat_inSiteNameText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outSiteNameText] = 0xff55ab4f;
        defaultColors[key_chat_outSiteNameText] = 0xffffffff;
        defaultColors[key_chat_inContactNameText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outContactNameText] = 0xff55ab4f;
        defaultColors[key_chat_outContactNameText] = 0xffffffff;
        defaultColors[key_chat_inContactPhoneText] = 0xff2f3438;
        defaultColors[key_chat_inContactPhoneSelectedText] = 0xff2f3438;
        defaultColors[key_chat_outContactPhoneText] = 0xff354234;
        defaultColors[key_chat_outContactPhoneSelectedText] = 0xff354234;
        defaultColors[key_chat_mediaProgress] = 0xffffffff;
        defaultColors[key_chat_inAudioProgress] = 0xffffffff;
        defaultColors[key_chat_outAudioProgress] = 0xffefffde;
        defaultColors[key_chat_inAudioSelectedProgress] = 0xffeff8fe;
        defaultColors[key_chat_outAudioSelectedProgress] = 0xffe1f8cf;
        defaultColors[key_chat_mediaTimeText] = 0xffffffff;
        defaultColors[key_chat_inAdminText] = 0xffc0c6cb;
        defaultColors[key_chat_inAdminSelectedText] = 0xff89b4c1;
        defaultColors[key_chat_outAdminText] = 0xff70b15c;
        defaultColors[key_chat_outAdminSelectedText] = 0xff70b15c;
        defaultColors[key_chat_inTimeText] = 0xffa1aab3;
        defaultColors[key_chat_inTimeSelectedText] = 0xffa1aab3;
//        defaultColors[key_chat_outTimeText] = 0xff70b15c;
        defaultColors[key_chat_outTimeText] = 0xffa1aab3;
//        defaultColors[key_chat_outTimeSelectedText] = 0xff70b15c;
        defaultColors[key_chat_outTimeSelectedText] = 0xffa1aab3;
        defaultColors[key_chat_inAudioPerformerText] = 0xff2f3438;
        defaultColors[key_chat_inAudioPerformerSelectedText] = 0xff2f3438;
        defaultColors[key_chat_outAudioPerformerText] = 0xff354234;
        defaultColors[key_chat_outAudioPerformerSelectedText] = 0xff354234;
        defaultColors[key_chat_inAudioTitleText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outAudioTitleText] = 0xff55ab4f;
        defaultColors[key_chat_outAudioTitleText] = 0xffffffff;//audio
        defaultColors[key_chat_inAudioDurationText] = 0xffa1aab3;
//        defaultColors[key_chat_outAudioDurationText] = 0xff65b05b;
        defaultColors[key_chat_outAudioDurationText] = 0xffffffff;//audio
        defaultColors[key_chat_inAudioDurationSelectedText] = 0xff89b4c1;
//        defaultColors[key_chat_outAudioDurationSelectedText] = 0xff65b05b;
        defaultColors[key_chat_outAudioDurationSelectedText] = 0xffffffff;//audio
        defaultColors[key_chat_inAudioSeekbar] = 0xffe4eaf0;
        defaultColors[key_chat_inAudioCacheSeekbar] = 0x3fe4eaf0;
//        defaultColors[key_chat_outAudioSeekbar] = 0xffbbe3ac;
        defaultColors[key_chat_outAudioSeekbar] =  0xffffffff;//audio
//        defaultColors[key_chat_outAudioCacheSeekbar] = 0x3fbbe3ac;
        defaultColors[key_chat_outAudioCacheSeekbar] =  0xffdee5eb;//audio
        defaultColors[key_chat_inAudioSeekbarSelected] = 0xffbcdee8;
//        defaultColors[key_chat_outAudioSeekbarSelected] = 0xffa9dd96;
        defaultColors[key_chat_outAudioSeekbarSelected] =  0xffdee5eb;//audio
        defaultColors[key_chat_inAudioSeekbarFill] = 0xff093489;//appBlue
//        defaultColors[key_chat_outAudioSeekbarFill] = 0xff78c272;
        defaultColors[key_chat_outAudioSeekbarFill] = 0xffffffff;//audio
        defaultColors[key_chat_inVoiceSeekbar] = 0xffdee5eb;
//        defaultColors[key_chat_outVoiceSeekbar] = 0xffbbe3ac;
        defaultColors[key_chat_outVoiceSeekbar] = 0xffdee5eb;//voice
        defaultColors[key_chat_inVoiceSeekbarSelected] = 0xffbcdee8;
//        defaultColors[key_chat_outVoiceSeekbarSelected] = 0xffa9dd96;
        defaultColors[key_chat_outVoiceSeekbarSelected] = 0xffdee5eb;//voice
        defaultColors[key_chat_inVoiceSeekbarFill] = 0xff093489;//appBlue
//        defaultColors[key_chat_outVoiceSeekbarFill] = 0xff78c272;
        defaultColors[key_chat_outVoiceSeekbarFill] = 0xffffffff;//voice
        defaultColors[key_chat_inFileProgress] = 0xffebf0f5;
        defaultColors[key_chat_outFileProgress] = 0xffdaf5c3;
        defaultColors[key_chat_inFileProgressSelected] = 0xffcbeaf6;
        defaultColors[key_chat_outFileProgressSelected] = 0xffc5eca7;
        defaultColors[key_chat_inFileNameText] = 0xff093489;//appBlue
//        defaultColors[key_chat_outFileNameText] = 0xff55ab4f;
        defaultColors[key_chat_outFileNameText] = 0xffffffff; //file
        defaultColors[key_chat_inFileInfoText] = 0xffa1aab3;
//        defaultColors[key_chat_outFileInfoText] = 0xff65b05b;
        defaultColors[key_chat_outFileInfoText] = 0xffffffff; //file
        defaultColors[key_chat_inFileInfoSelectedText] = 0xff89b4c1;
//        defaultColors[key_chat_outFileInfoSelectedText] = 0xff65b05b;
        defaultColors[key_chat_outFileInfoSelectedText] = 0xffffffff; //file
        defaultColors[key_chat_inFileBackground] = 0xffebf0f5;
        defaultColors[key_chat_outFileBackground] = 0xffdaf5c3;
        defaultColors[key_chat_inFileBackgroundSelected] = 0xffcbeaf6;
        defaultColors[key_chat_outFileBackgroundSelected] = 0xffc5eca7;
        defaultColors[key_chat_inVenueInfoText] = 0xffa1aab3;
//        defaultColors[key_chat_outVenueInfoText] = 0xff65b05b;
        defaultColors[key_chat_outVenueInfoText] = 0xffffffff; //venueInfo
        defaultColors[key_chat_inVenueInfoSelectedText] = 0xff89b4c1;
//        defaultColors[key_chat_outVenueInfoSelectedText] = 0xff65b05b;
        defaultColors[key_chat_outVenueInfoSelectedText] = 0xffffffff; //venueInfo
        defaultColors[key_chat_mediaInfoText] = 0xffffffff;
        defaultColors[key_chat_linkSelectBackground] = 0x3362a9e3;
        defaultColors[key_chat_outLinkSelectBackground] = 0x3362a9e3;
        defaultColors[key_chat_textSelectBackground] = 0x6662a9e3;
        defaultColors[key_chat_emojiPanelBackground] = 0xfff0f2f5;
        defaultColors[key_chat_emojiSearchBackground] = 0xffe5e9ee;
        defaultColors[key_chat_emojiSearchIcon] = 0xff94a1af;
        defaultColors[key_chat_emojiPanelShadowLine] = 0x12000000;
        defaultColors[key_chat_emojiPanelEmptyText] = 0xff949ba1;
        defaultColors[key_chat_emojiPanelIcon] = 0xff9da4ab;
        defaultColors[key_chat_emojiBottomPanelIcon] = 0xff8c9197;
        defaultColors[key_chat_emojiPanelIconSelected] = 0xff093489;//appBlue
        defaultColors[key_chat_emojiPanelStickerPackSelector] = 0xffe2e5e7;
        defaultColors[key_chat_emojiPanelStickerPackSelectorLine] = 0xff093489;//appBlue
        defaultColors[key_chat_emojiPanelBackspace] = 0xff8c9197;
        defaultColors[key_chat_emojiPanelTrendingTitle] = 0xff222222;
        defaultColors[key_chat_emojiPanelStickerSetName] = 0xff828b94;
        defaultColors[key_chat_emojiPanelStickerSetNameHighlight] = 0xff093489;//appBlue
        defaultColors[key_chat_emojiPanelStickerSetNameIcon] = 0xffb1b6bc;
        defaultColors[key_chat_emojiPanelTrendingDescription] = 0xff8a8a8a;
        defaultColors[key_chat_botKeyboardButtonText] = 0xff36474f;
        defaultColors[key_chat_botKeyboardButtonBackground] = 0xffe4e7e9;
        defaultColors[key_chat_botKeyboardButtonBackgroundPressed] = 0xffccd1d4;
        defaultColors[key_chat_unreadMessagesStartArrowIcon] = 0xffa2b5c7;
        defaultColors[key_chat_unreadMessagesStartText] = 0xff093489;//appBlue
        defaultColors[key_chat_unreadMessagesStartBackground] = 0xffffffff;
        defaultColors[key_chat_inLocationBackground] = 0xffebf0f5;
        defaultColors[key_chat_inLocationIcon] = 0xffa2b5c7;
        defaultColors[key_chat_outLocationIcon] = 0xff87bf78;
        defaultColors[key_chat_inContactBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_inContactIcon] = 0xffffffff;
//        defaultColors[key_chat_outContactBackground] = 0xff78c272;
        defaultColors[key_chat_outContactBackground] = 0xffffffff;
        defaultColors[key_chat_outContactIcon] = 0xffefffde;
        defaultColors[key_chat_searchPanelIcons] = 0xff676a6f;
        defaultColors[key_chat_searchPanelText] = 0xff676a6f;
        defaultColors[key_chat_secretChatStatusText] = 0xff7f7f7f;
        defaultColors[key_chat_fieldOverlayText] = 0xff093489;//appBlue
        defaultColors[key_chat_stickersHintPanel] = 0xffffffff;
        defaultColors[key_chat_replyPanelIcons] = 0xff093489;//appBlue
        defaultColors[key_chat_replyPanelClose] = 0xff8e959b;
        defaultColors[key_chat_replyPanelName] = 0xff093489;//appBlue
        defaultColors[key_chat_replyPanelLine] = 0xffe8e8e8;
        defaultColors[key_chat_messagePanelBackground] = 0xffffffff;
        defaultColors[key_chat_messagePanelText] = 0xff000000;
        defaultColors[key_chat_messagePanelHint] = 0xffa4acb3;
        defaultColors[key_chat_messagePanelCursor] = 0xff093489;//appBlue
        defaultColors[key_chat_messagePanelShadow] = 0xff000000;
        defaultColors[key_chat_messagePanelIcons] = 0xff8e959b;
        defaultColors[key_chat_recordedVoicePlayPause] = 0xffffffff;
        defaultColors[key_chat_recordedVoiceDot] = 0xffda564d;
        defaultColors[key_chat_recordedVoiceBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_recordedVoiceProgress] = 0xff093489;//appBlue
        defaultColors[key_chat_recordedVoiceProgressInner] = 0xffffffff;
        defaultColors[key_chat_recordVoiceCancel] = 0xff3A95D4;
        defaultColors[key_chat_messagePanelSend] = 0xff093489;//appBlue
        defaultColors[key_chat_messagePanelVoiceLock] = 0xffa4a4a4;
        defaultColors[key_chat_messagePanelVoiceLockBackground] = 0xffffffff;
        defaultColors[key_chat_messagePanelVoiceLockShadow] = 0xff000000;
        defaultColors[key_chat_recordTime] = 0xff8e959b;
        defaultColors[key_chat_emojiPanelNewTrending] = 0xff093489;//appBlue
        defaultColors[key_chat_gifSaveHintText] = 0xffffffff;
        defaultColors[key_chat_gifSaveHintBackground] = 0xcc111111;
        defaultColors[key_chat_goDownButton] = 0xffffffff;
        defaultColors[key_chat_goDownButtonIcon] = 0xff8e959b;
        defaultColors[key_chat_goDownButtonCounter] = 0xffffffff;
        defaultColors[key_chat_goDownButtonCounterBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_messagePanelCancelInlineBot] = 0xffadadad;
        defaultColors[key_chat_messagePanelVoicePressed] = 0xffffffff;
        defaultColors[key_chat_messagePanelVoiceBackground] = 0xff093489;//appBlue
        defaultColors[key_chat_messagePanelVoiceDelete] = 0xff737373;
        defaultColors[key_chat_messagePanelVoiceDuration] = 0xffffffff;
        defaultColors[key_chat_inlineResultIcon] = 0xff093489;//appBlue
        defaultColors[key_chat_topPanelBackground] = 0xffffffff;
        defaultColors[key_chat_topPanelClose] = 0xff8b969b;
        defaultColors[key_chat_topPanelLine] =0xff093489;//appBlue
        defaultColors[key_chat_topPanelTitle] = 0xff093489;//appBlue
        defaultColors[key_chat_topPanelMessage] = 0xff878e91;
        defaultColors[key_chat_addContact] = 0xff093489;//appBlue
        defaultColors[key_chat_inLoader] = 0xff093489;//appBlue
        defaultColors[key_chat_inLoaderSelected] = 0xff093489;//appBlue
//        defaultColors[key_chat_outLoader] = 0xff78c272;
        defaultColors[key_chat_outLoader] =  0xffffffff;//voice play btn
//        defaultColors[key_chat_outLoaderSelected] = 0xff6ab564;
        defaultColors[key_chat_outLoaderSelected] =  0xffffffff;//voice play btn
        defaultColors[key_chat_inLoaderPhoto] = 0xffa2b8c8;
        defaultColors[key_chat_mediaLoaderPhoto] = 0x66000000;
        defaultColors[key_chat_mediaLoaderPhotoSelected] = 0x7f000000;
        defaultColors[key_chat_mediaLoaderPhotoIcon] = 0xffffffff;
        defaultColors[key_chat_mediaLoaderPhotoIconSelected] = 0xffd9d9d9;
        defaultColors[key_chat_serviceBackgroundSelector] = 0x20ffffff;

        defaultColors[key_profile_creatorIcon] = 0xff093489;//appBlue
        defaultColors[key_profile_actionIcon] = 0xff81868a;
        defaultColors[key_profile_actionBackground] = 0xffffffff;
        defaultColors[key_profile_actionPressedBackground] = 0xfff2f2f2;
        defaultColors[key_profile_verifiedBackground] = 0xffb2d6f8;
        defaultColors[key_profile_verifiedCheck] = 0xff093489;//appBlue
        defaultColors[key_profile_title] = 0xffffffff;
        defaultColors[key_profile_status] = 0xffd7eafa;

        defaultColors[key_profile_tabText] = 0xff878c90;
        defaultColors[key_profile_tabSelectedText] = 0xff093489;//appBlue
        defaultColors[key_profile_tabSelectedLine] = 0xff093489;//appBlue
        defaultColors[key_profile_tabSelector] = 0x0f000000;

        defaultColors[key_player_actionBarSelector] = 0x0f000000;
        defaultColors[key_player_actionBarTitle] = 0xff2f3438;
        defaultColors[key_player_actionBarSubtitle] = 0xff8a8a8a;
        defaultColors[key_player_actionBarItems] = 0xff8a8a8a;
        defaultColors[key_player_background] = 0xffffffff;
        defaultColors[key_player_time] = 0xff8c9296;
        defaultColors[key_player_progressBackground] = 0xffEBEDF0;
        defaultColors[key_player_progressCachedBackground] = 0xffC5DCF0;
        defaultColors[key_player_progress] = 0xff093489;//appBlue
        defaultColors[key_player_button] = 0xff333333;
        defaultColors[key_player_buttonActive] = 0xff093489;//appBlue

        defaultColors[key_sheet_scrollUp] = 0xffe1e4e8;
        defaultColors[key_sheet_other] = 0xffc9cdd3;

        defaultColors[key_files_folderIcon] = 0xffffffff;
        defaultColors[key_files_folderIconBackground] = 0xff5dafeb;
        defaultColors[key_files_iconText] = 0xffffffff;

        defaultColors[key_sessions_devicesImage] = 0xff969696;

        defaultColors[key_passport_authorizeBackground] = 0xff093489;//appBlue
        defaultColors[key_passport_authorizeBackgroundSelected] =0xff093489;//appBlue
        defaultColors[key_passport_authorizeText] = 0xffffffff;

        defaultColors[key_location_sendLocationBackground] =  0xff093489;//appBlue
        defaultColors[key_location_sendLocationIcon] = 0xffffffff;
        defaultColors[key_location_sendLocationText] = 0xff1c8ad8;
        defaultColors[key_location_sendLiveLocationBackground] = 0xff4fc244;
        defaultColors[key_location_sendLiveLocationIcon] = 0xffffffff;
        defaultColors[key_location_sendLiveLocationText] = 0xff36ab24;
        defaultColors[key_location_liveLocationProgress] = 0xff093489;//appBlue
        defaultColors[key_location_placeLocationBackground] = 0xff093489;//appBlue
        defaultColors[key_location_actionIcon] = 0xff3a4045;
        defaultColors[key_location_actionActiveIcon] = 0xff4290e6;
        defaultColors[key_location_actionBackground] = 0xffffffff;
        defaultColors[key_location_actionPressedBackground] = 0xfff2f2f2;

        defaultColors[key_dialog_liveLocationProgress] = 0xff093489;//appBlue

        defaultColors[key_calls_callReceivedGreenIcon] = 0xff00c853;
        defaultColors[key_calls_callReceivedRedIcon] = 0xffff4848;

        defaultColors[key_featuredStickers_addedIcon] = 0xff093489;//appBlue
        defaultColors[key_featuredStickers_buttonProgress] = 0xffffffff;
        defaultColors[key_featuredStickers_addButton] = 0xff093489;//appBlue
        defaultColors[key_featuredStickers_addButtonPressed] = 0xff093489;//appBlue
        defaultColors[key_featuredStickers_removeButtonText] = 0xff5093d3;
        defaultColors[key_featuredStickers_buttonText] = 0xffffffff;
        defaultColors[key_featuredStickers_unread] = 0xff093489;//appBlue

        defaultColors[key_inappPlayerPerformer] = 0xff2f3438;
        defaultColors[key_inappPlayerTitle] = 0xff2f3438;
        defaultColors[key_inappPlayerBackground] = 0xffffffff;
        defaultColors[key_inappPlayerPlayPause] =0xff093489;//appBlue
        defaultColors[key_inappPlayerClose] = 0xff8b969b;

        defaultColors[key_returnToCallBackground] = 0xff093489;//appBlue
        defaultColors[key_returnToCallMutedBackground] = 0xff9DA7B1;
        defaultColors[key_returnToCallText] = 0xffffffff;

        defaultColors[key_sharedMedia_startStopLoadIcon] = 0xff093489;//appBlue
        defaultColors[key_sharedMedia_linkPlaceholder] = 0xfff0f3f5;
        defaultColors[key_sharedMedia_linkPlaceholderText] = 0xffb7bec3;
        defaultColors[key_sharedMedia_photoPlaceholder] = 0xffedf3f7;

        defaultColors[key_checkbox] = 0xff5ec245;
        defaultColors[key_checkboxCheck] = 0xffffffff;
        defaultColors[key_checkboxDisabled] = 0xffb0b9c2;

        defaultColors[key_stickers_menu] = 0xffb6bdc5;
        defaultColors[key_stickers_menuSelector] = 0x0f000000;

        defaultColors[key_changephoneinfo_image2] =0xff093489;//appBlue

        defaultColors[key_groupcreate_hintText] = 0xffa1aab3;
        defaultColors[key_groupcreate_cursor] =0xff093489;//appBlue
        defaultColors[key_groupcreate_sectionShadow] = 0xff000000;
        defaultColors[key_groupcreate_sectionText] = 0xff7c8288;
        defaultColors[key_groupcreate_spanText] = 0xff222222;
        defaultColors[key_groupcreate_spanBackground] = 0xfff2f2f2;
        defaultColors[key_groupcreate_spanDelete] = 0xffffffff;

        defaultColors[key_contacts_inviteBackground] = 0xff55be61;
        defaultColors[key_contacts_inviteText] = 0xffffffff;

        defaultColors[key_login_progressInner] = 0xffe1eaf2;
        defaultColors[key_login_progressOuter] =0xff093489;//appBlue

        defaultColors[key_picker_enabledButton] = 0xff093489;//appBlue
        defaultColors[key_picker_disabledButton] = 0xff999999;
        defaultColors[key_picker_badge] = 0xff093489;//appBlue
        defaultColors[key_picker_badgeText] = 0xffffffff;

        defaultColors[key_chat_botSwitchToInlineText] = 0xff093489;//appBlue

        defaultColors[key_undo_background] = 0xea272f38;
        defaultColors[key_undo_cancelColor] = 0xff093489;//appBlue
        defaultColors[key_undo_infoColor] = 0xffffffff;

        defaultColors[key_chat_outTextSelectionHighlight] = 0x2E3F9923;
        defaultColors[key_chat_inTextSelectionHighlight] = 0x5062A9E3;
        defaultColors[key_chat_TextSelectionCursor] = 0xFF419FE8;
        defaultColors[key_chat_outTextSelectionCursor] = 0xFF419FE8;
        defaultColors[key_chat_outBubbleLocationPlaceholder] = 0x1e307311;
        defaultColors[key_chat_inBubbleLocationPlaceholder] = 0x1e506373;
        defaultColors[key_chat_BlurAlpha] = 0xFF000000;

        defaultColors[key_statisticChartSignature] = 0x7f252529;
        defaultColors[key_statisticChartSignatureAlpha] = 0x7f252529;
        defaultColors[key_statisticChartHintLine] = 0x1a182D3B;
        defaultColors[key_statisticChartActiveLine] = 0x33000000;
        defaultColors[key_statisticChartInactivePickerChart] = 0x99e2eef9;
        defaultColors[key_statisticChartActivePickerChart] = 0xd8baccd9;

        defaultColors[key_statisticChartRipple] = 0x2c7e9db7;
        defaultColors[key_statisticChartBackZoomColor] = 0xff108BE3;
        defaultColors[key_statisticChartChevronColor] = 0xffD2D5D7;

        defaultColors[key_statisticChartLine_blue] = 0xff327FE5;
        defaultColors[key_statisticChartLine_green] = 0xff61C752;
        defaultColors[key_statisticChartLine_red] = 0xffE05356;
        defaultColors[key_statisticChartLine_golden] = 0xffEBA52D;
        defaultColors[key_statisticChartLine_lightblue] = 0xff58A8ED;
        defaultColors[key_statisticChartLine_lightgreen] = 0xff8FCF39;
        defaultColors[key_statisticChartLine_orange] = 0xffF28C39;
        defaultColors[key_statisticChartLine_indigo] = 0xff7F79F3;
        defaultColors[key_statisticChartLine_purple] = 0xff9F79E8;
        defaultColors[key_statisticChartLine_cyan] = 0xff40D0CA;
        defaultColors[key_statisticChartLineEmpty] = 0xFFEEEEEE;

        defaultColors[key_color_blue] = 0xff327FE5;
        defaultColors[key_color_green] = 0xff61C752;
        defaultColors[key_color_red] = 0xffE05356;
        defaultColors[key_color_yellow] = 0xffEBA52D;
        defaultColors[key_color_lightblue] = 0xff58A8ED;
        defaultColors[key_color_lightgreen] = 0xff8FCF39;
        defaultColors[key_color_orange] = 0xffF28C39;
        defaultColors[key_color_purple] = 0xff9F79E8;
        defaultColors[key_color_cyan] = 0xff40D0CA;

        defaultColors[key_voipgroup_checkMenu] = 0xff6BB6F9;
        defaultColors[key_voipgroup_muteButton] = 0xff77E55C;
        defaultColors[key_voipgroup_muteButton2] = 0xff7DDCAA;
        defaultColors[key_voipgroup_muteButton3] = 0xff56C7FE;
        defaultColors[key_voipgroup_searchText] = 0xffffffff;
        defaultColors[key_voipgroup_searchPlaceholder] = 0xff858D94;
        defaultColors[key_voipgroup_searchBackground] = 0xff303B47;
        defaultColors[key_voipgroup_leaveCallMenu] = 0xffFF7575;
        defaultColors[key_voipgroup_scrollUp] = 0xff394654;
        defaultColors[key_voipgroup_soundButton] = 0x7d2C414D;
        defaultColors[key_voipgroup_soundButtonActive] = 0x7d22A4EB;
        defaultColors[key_voipgroup_soundButtonActiveScrolled] = 0x8233B4FF;
        defaultColors[key_voipgroup_soundButton2] = 0x7d28593A;
        defaultColors[key_voipgroup_soundButtonActive2] = 0x7d18B751;
        defaultColors[key_voipgroup_soundButtonActive2Scrolled] = 0x8224BF46;
        defaultColors[key_voipgroup_leaveButton] = 0x7dF75C5C;
        defaultColors[key_voipgroup_leaveButtonScrolled] = 0x82D14D54;
        defaultColors[key_voipgroup_connectingProgress] = 0xff28BAFF;
        defaultColors[key_voipgroup_disabledButton] = 0xff1C2229;
        defaultColors[key_voipgroup_disabledButtonActive] = 0xff2C3A45;
        defaultColors[key_voipgroup_disabledButtonActiveScrolled] = 0x8277A1FC;
        defaultColors[key_voipgroup_unmuteButton] = 0xff539EF8;
        defaultColors[key_voipgroup_unmuteButton2] = 0xff66D4FB;
        defaultColors[key_voipgroup_actionBarUnscrolled] = 0xff191F26;
        defaultColors[key_voipgroup_listViewBackgroundUnscrolled] = 0xff222A33;
        defaultColors[key_voipgroup_lastSeenTextUnscrolled] = 0xff858D94;
        defaultColors[key_voipgroup_mutedIconUnscrolled] = 0xff7E868C;
        defaultColors[key_voipgroup_actionBar] = 0xff0F1317;
        defaultColors[key_voipgroup_actionBarItems] = 0xffffffff;
        defaultColors[key_voipgroup_actionBarItemsSelector] = 0x1eBADBFF;
        defaultColors[key_voipgroup_mutedByAdminIcon] = 0xffFF7070;
        defaultColors[key_voipgroup_mutedIcon] = 0xff6F7980;
        defaultColors[key_voipgroup_lastSeenText] = 0xff79838A;
        defaultColors[key_voipgroup_nameText] = 0xffffffff;
        defaultColors[key_voipgroup_listViewBackground] = 0xff1C2229;
        defaultColors[key_voipgroup_dialogBackground] = 0xff1C2229;
        defaultColors[key_voipgroup_listeningText] = 0xff4DB8FF;
        defaultColors[key_voipgroup_speakingText] = 0xff77EE7D;
        defaultColors[key_voipgroup_listSelector] = 0x0effffff;
        defaultColors[key_voipgroup_inviteMembersBackground] = 0xff222A33;
        defaultColors[key_voipgroup_overlayBlue1] = 0xff2BCEFF;
        defaultColors[key_voipgroup_overlayBlue2] = 0xff0976E3;
        defaultColors[key_voipgroup_overlayGreen1] = 0xff12B522;
        defaultColors[key_voipgroup_overlayGreen2] = 0xff00D6C1;
        defaultColors[key_voipgroup_topPanelBlue1] = 0xff60C7FB;
        defaultColors[key_voipgroup_topPanelBlue2] = 0xff519FF9;
        defaultColors[key_voipgroup_topPanelGreen1] = 0xff52CE5D;
        defaultColors[key_voipgroup_topPanelGreen2] = 0xff00B1C0;
        defaultColors[key_voipgroup_topPanelGray] = 0xff8599aa;

        defaultColors[key_voipgroup_overlayAlertGradientMuted] = 0xff236D92;
        defaultColors[key_voipgroup_overlayAlertGradientMuted2] = 0xff2C4D6B;
        defaultColors[key_voipgroup_overlayAlertGradientUnmuted] = 0xff0C8A8C;
        defaultColors[key_voipgroup_overlayAlertGradientUnmuted2] = 0xff284C75;
        defaultColors[key_voipgroup_mutedByAdminGradient] = 0xff57A4FE;
        defaultColors[key_voipgroup_mutedByAdminGradient2] = 0xffF05459;
        defaultColors[key_voipgroup_mutedByAdminGradient3] = 0xff766EE9;
        defaultColors[key_voipgroup_overlayAlertMutedByAdmin] = 0xff67709E;
        defaultColors[key_voipgroup_overlayAlertMutedByAdmin2] = 0xff2F5078;
        defaultColors[key_voipgroup_mutedByAdminMuteButton] = 0x7F78A3FF;
        defaultColors[key_voipgroup_mutedByAdminMuteButtonDisabled] = 0x3378A3FF;
        defaultColors[key_voipgroup_windowBackgroundWhiteInputField] = 0xffdbdbdb;
        defaultColors[key_voipgroup_windowBackgroundWhiteInputFieldActivated] = 0xff37a9f0;

        defaultColors[key_chat_outReactionButtonBackground] =0xffffffff;//inReactionBackgraund
        defaultColors[key_chat_inReactionButtonBackground] = 0xff72b5e8;
        defaultColors[key_chat_inReactionButtonText] = 0xff3a8ccf;
//        defaultColors[key_chat_outReactionButtonText] = 0xff55ab4f;
        defaultColors[key_chat_outReactionButtonText] =0xffffffff;
        defaultColors[key_chat_inReactionButtonTextSelected] = 0xffffffff;
        defaultColors[key_chat_outReactionButtonTextSelected] = 0xffffffff;

        defaultColors[key_premiumGradient0] = 0xff4ACD43;
        defaultColors[key_premiumGradient1] = 0xff55A5FF;
        defaultColors[key_premiumGradient2] = 0xffA767FF;
        defaultColors[key_premiumGradient3] = 0xffDB5C9D;
        defaultColors[key_premiumGradient4] = 0xffF38926;

        defaultColors[key_premiumGradientBackground1] = 0xff55A5FF;
        defaultColors[key_premiumGradientBackground2] = 0xffA767FF;
        defaultColors[key_premiumGradientBackground3] = 0xffDB5C9D;
        defaultColors[key_premiumGradientBackground4] = 0xffF38926;
        defaultColors[key_premiumGradientBackgroundOverlay] = Color.WHITE;
        defaultColors[key_premiumStartGradient1] = 0xffFFFFFF;
        defaultColors[key_premiumStartGradient2] = 0xffE3ECFA;
        defaultColors[key_premiumStartSmallStarsColor] = ColorUtils.setAlphaComponent(Color.WHITE, 90);
        defaultColors[key_premiumStartSmallStarsColor2] = ColorUtils.setAlphaComponent(Color.WHITE, 90);
        defaultColors[key_premiumGradientBottomSheet1] = 0xff5B9DE7;
        defaultColors[key_premiumGradientBottomSheet2] = 0xffAB87DD;
        defaultColors[key_premiumGradientBottomSheet3] = 0xffE794BE;
        defaultColors[key_topics_unreadCounter] = 0xff4ecc5e;
        defaultColors[key_topics_unreadCounterMuted] = 0xff8b8d8f;

        return defaultColors;
    }

    public static SparseArray<String> createColorKeysMap() {
        SparseArray<String> colorKeysMap = new SparseArray<>();
        colorKeysMap.put(key_wallpaperFileOffset, "wallpaperFileOffset");
        colorKeysMap.put(key_dialogBackground, "dialogBackground");
        colorKeysMap.put(key_dialogBackgroundGray, "dialogBackgroundGray");
        colorKeysMap.put(key_dialogTextBlack, "dialogTextBlack");
        colorKeysMap.put(key_dialogTextLink, "dialogTextLink");
        colorKeysMap.put(key_dialogLinkSelection, "dialogLinkSelection");
        colorKeysMap.put(key_dialogTextBlue, "dialogTextBlue");
        colorKeysMap.put(key_dialogTextBlue2, "dialogTextBlue2");
        colorKeysMap.put(key_dialogTextBlue4, "dialogTextBlue4");
        colorKeysMap.put(key_dialogTextGray, "dialogTextGray");
        colorKeysMap.put(key_dialogTextGray2, "dialogTextGray2");
        colorKeysMap.put(key_dialogTextGray3, "dialogTextGray3");
        colorKeysMap.put(key_dialogTextGray4, "dialogTextGray4");
        colorKeysMap.put(key_dialogTextHint, "dialogTextHint");
        colorKeysMap.put(key_dialogInputField, "dialogInputField");
        colorKeysMap.put(key_dialogInputFieldActivated, "dialogInputFieldActivated");
        colorKeysMap.put(key_dialogCheckboxSquareBackground, "dialogCheckboxSquareBackground");
        colorKeysMap.put(key_dialogCheckboxSquareCheck, "dialogCheckboxSquareCheck");
        colorKeysMap.put(key_dialogCheckboxSquareUnchecked, "dialogCheckboxSquareUnchecked");
        colorKeysMap.put(key_dialogCheckboxSquareDisabled, "dialogCheckboxSquareDisabled");
        colorKeysMap.put(key_dialogScrollGlow, "dialogScrollGlow");
        colorKeysMap.put(key_dialogRoundCheckBox, "dialogRoundCheckBox");
        colorKeysMap.put(key_dialogRoundCheckBoxCheck, "dialogRoundCheckBoxCheck");
        colorKeysMap.put(key_dialogRadioBackground, "dialogRadioBackground");
        colorKeysMap.put(key_dialogRadioBackgroundChecked, "dialogRadioBackgroundChecked");
        colorKeysMap.put(key_dialogLineProgress, "dialogLineProgress");
        colorKeysMap.put(key_dialogLineProgressBackground, "dialogLineProgressBackground");
        colorKeysMap.put(key_dialogButton, "dialogButton");
        colorKeysMap.put(key_dialogButtonSelector, "dialogButtonSelector");
        colorKeysMap.put(key_dialogIcon, "dialogIcon");
        colorKeysMap.put(key_dialogGrayLine, "dialogGrayLine");
        colorKeysMap.put(key_dialogTopBackground, "dialogTopBackground");
        colorKeysMap.put(key_dialogCameraIcon, "dialogCameraIcon");
        colorKeysMap.put(key_dialog_inlineProgressBackground, "dialog_inlineProgressBackground");
        colorKeysMap.put(key_dialog_inlineProgress, "dialog_inlineProgress");
        colorKeysMap.put(key_dialogSearchBackground, "dialogSearchBackground");
        colorKeysMap.put(key_dialogSearchHint, "dialogSearchHint");
        colorKeysMap.put(key_dialogSearchIcon, "dialogSearchIcon");
        colorKeysMap.put(key_dialogSearchText, "dialogSearchText");
        colorKeysMap.put(key_dialogFloatingButton, "dialogFloatingButton");
        colorKeysMap.put(key_dialogFloatingButtonPressed, "dialogFloatingButtonPressed");
        colorKeysMap.put(key_dialogFloatingIcon, "dialogFloatingIcon");
        colorKeysMap.put(key_dialogShadowLine, "dialogShadowLine");
        colorKeysMap.put(key_dialogEmptyImage, "dialogEmptyImage");
        colorKeysMap.put(key_dialogEmptyText, "dialogEmptyText");
        colorKeysMap.put(key_dialogSwipeRemove, "dialogSwipeRemove");
        colorKeysMap.put(key_dialogReactionMentionBackground, "dialogReactionMentionBackground");
        colorKeysMap.put(key_windowBackgroundWhite, "windowBackgroundWhite");
        colorKeysMap.put(key_windowBackgroundUnchecked, "windowBackgroundUnchecked");
        colorKeysMap.put(key_windowBackgroundChecked, "windowBackgroundChecked");
        colorKeysMap.put(key_windowBackgroundCheckText, "windowBackgroundCheckText");
        colorKeysMap.put(key_progressCircle, "progressCircle");
        colorKeysMap.put(key_listSelector, "listSelectorSDK21");
        colorKeysMap.put(key_windowBackgroundWhiteInputField, "windowBackgroundWhiteInputField");
        colorKeysMap.put(key_windowBackgroundWhiteInputFieldActivated, "windowBackgroundWhiteInputFieldActivated");
        colorKeysMap.put(key_windowBackgroundWhiteGrayIcon, "windowBackgroundWhiteGrayIcon");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText, "windowBackgroundWhiteBlueText");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText2, "windowBackgroundWhiteBlueText2");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText3, "windowBackgroundWhiteBlueText3");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText4, "windowBackgroundWhiteBlueText4");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText5, "windowBackgroundWhiteBlueText5");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText6, "windowBackgroundWhiteBlueText6");
        colorKeysMap.put(key_windowBackgroundWhiteBlueText7, "windowBackgroundWhiteBlueText7");
        colorKeysMap.put(key_windowBackgroundWhiteBlueButton, "windowBackgroundWhiteBlueButton");
        colorKeysMap.put(key_windowBackgroundWhiteBlueIcon, "windowBackgroundWhiteBlueIcon");
        colorKeysMap.put(key_windowBackgroundWhiteGreenText, "windowBackgroundWhiteGreenText");
        colorKeysMap.put(key_windowBackgroundWhiteGreenText2, "windowBackgroundWhiteGreenText2");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText, "windowBackgroundWhiteGrayText");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText2, "windowBackgroundWhiteGrayText2");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText3, "windowBackgroundWhiteGrayText3");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText4, "windowBackgroundWhiteGrayText4");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText5, "windowBackgroundWhiteGrayText5");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText6, "windowBackgroundWhiteGrayText6");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText7, "windowBackgroundWhiteGrayText7");
        colorKeysMap.put(key_windowBackgroundWhiteGrayText8, "windowBackgroundWhiteGrayText8");
        colorKeysMap.put(key_windowBackgroundWhiteBlackText, "windowBackgroundWhiteBlackText");
        colorKeysMap.put(key_windowBackgroundWhiteHintText, "windowBackgroundWhiteHintText");
        colorKeysMap.put(key_windowBackgroundWhiteValueText, "windowBackgroundWhiteValueText");
        colorKeysMap.put(key_windowBackgroundWhiteLinkText, "windowBackgroundWhiteLinkText");
        colorKeysMap.put(key_windowBackgroundWhiteLinkSelection, "windowBackgroundWhiteLinkSelection");
        colorKeysMap.put(key_windowBackgroundWhiteBlueHeader, "windowBackgroundWhiteBlueHeader");
        colorKeysMap.put(key_switchTrack, "switchTrack");
        colorKeysMap.put(key_switchTrackChecked, "switchTrackChecked");
        colorKeysMap.put(key_switchTrackBlue, "switchTrackBlue");
        colorKeysMap.put(key_switchTrackBlueChecked, "switchTrackBlueChecked");
        colorKeysMap.put(key_switchTrackBlueThumb, "switchTrackBlueThumb");
        colorKeysMap.put(key_switchTrackBlueThumbChecked, "switchTrackBlueThumbChecked");
        colorKeysMap.put(key_switchTrackBlueSelector, "switchTrackBlueSelector");
        colorKeysMap.put(key_switchTrackBlueSelectorChecked, "switchTrackBlueSelectorChecked");
        colorKeysMap.put(key_switch2Track, "switch2Track");
        colorKeysMap.put(key_switch2TrackChecked, "switch2TrackChecked");
        colorKeysMap.put(key_checkboxSquareBackground, "checkboxSquareBackground");
        colorKeysMap.put(key_checkboxSquareCheck, "checkboxSquareCheck");
        colorKeysMap.put(key_checkboxSquareUnchecked, "checkboxSquareUnchecked");
        colorKeysMap.put(key_checkboxSquareDisabled, "checkboxSquareDisabled");
        colorKeysMap.put(key_windowBackgroundGray, "windowBackgroundGray");
        colorKeysMap.put(key_windowBackgroundGrayShadow, "windowBackgroundGrayShadow");
        colorKeysMap.put(key_emptyListPlaceholder, "emptyListPlaceholder");
        colorKeysMap.put(key_divider, "divider");
        colorKeysMap.put(key_graySection, "graySection");
        colorKeysMap.put(key_graySectionText, "key_graySectionText");
        colorKeysMap.put(key_radioBackground, "radioBackground");
        colorKeysMap.put(key_radioBackgroundChecked, "radioBackgroundChecked");
        colorKeysMap.put(key_checkbox, "checkbox");
        colorKeysMap.put(key_checkboxDisabled, "checkboxDisabled");
        colorKeysMap.put(key_checkboxCheck, "checkboxCheck");
        colorKeysMap.put(key_fastScrollActive, "fastScrollActive");
        colorKeysMap.put(key_fastScrollInactive, "fastScrollInactive");
        colorKeysMap.put(key_fastScrollText, "fastScrollText");
        colorKeysMap.put(key_text_RedRegular, "text_RedRegular");
        colorKeysMap.put(key_text_RedBold, "text_RedBold");
        colorKeysMap.put(key_fill_RedNormal, "fill_RedNormal");
        colorKeysMap.put(key_fill_RedDark, "fill_RedDark");
        colorKeysMap.put(key_inappPlayerPerformer, "inappPlayerPerformer");
        colorKeysMap.put(key_inappPlayerTitle, "inappPlayerTitle");
        colorKeysMap.put(key_inappPlayerBackground, "inappPlayerBackground");
        colorKeysMap.put(key_inappPlayerPlayPause, "inappPlayerPlayPause");
        colorKeysMap.put(key_inappPlayerClose, "inappPlayerClose");
        colorKeysMap.put(key_returnToCallBackground, "returnToCallBackground");
        colorKeysMap.put(key_returnToCallMutedBackground, "returnToCallMutedBackground");
        colorKeysMap.put(key_returnToCallText, "returnToCallText");
        colorKeysMap.put(key_contextProgressInner1, "contextProgressInner1");
        colorKeysMap.put(key_contextProgressOuter1, "contextProgressOuter1");
        colorKeysMap.put(key_contextProgressInner2, "contextProgressInner2");
        colorKeysMap.put(key_contextProgressOuter2, "contextProgressOuter2");
        colorKeysMap.put(key_contextProgressInner3, "contextProgressInner3");
        colorKeysMap.put(key_contextProgressOuter3, "contextProgressOuter3");
        colorKeysMap.put(key_contextProgressInner4, "contextProgressInner4");
        colorKeysMap.put(key_contextProgressOuter4, "contextProgressOuter4");
        colorKeysMap.put(key_avatar_text, "avatar_text");
        colorKeysMap.put(key_avatar_backgroundSaved, "avatar_backgroundSaved");
        colorKeysMap.put(key_avatar_background2Saved, "avatar_background2Saved");
        colorKeysMap.put(key_avatar_backgroundArchived, "avatar_backgroundArchived");
        colorKeysMap.put(key_avatar_backgroundArchivedHidden, "avatar_backgroundArchivedHidden");
        colorKeysMap.put(key_avatar_backgroundRed, "avatar_backgroundRed");
        colorKeysMap.put(key_avatar_backgroundOrange, "avatar_backgroundOrange");
        colorKeysMap.put(key_avatar_backgroundViolet, "avatar_backgroundViolet");
        colorKeysMap.put(key_avatar_backgroundGreen, "avatar_backgroundGreen");
        colorKeysMap.put(key_avatar_backgroundCyan, "avatar_backgroundCyan");
        colorKeysMap.put(key_avatar_backgroundBlue, "avatar_backgroundBlue");
        colorKeysMap.put(key_avatar_backgroundPink, "avatar_backgroundPink");
        colorKeysMap.put(key_avatar_background2Red, "avatar_background2Red");
        colorKeysMap.put(key_avatar_background2Orange, "avatar_background2Orange");
        colorKeysMap.put(key_avatar_background2Violet, "avatar_background2Violet");
        colorKeysMap.put(key_avatar_background2Green, "avatar_background2Green");
        colorKeysMap.put(key_avatar_background2Cyan, "avatar_background2Cyan");
        colorKeysMap.put(key_avatar_background2Blue, "avatar_background2Blue");
        colorKeysMap.put(key_avatar_background2Pink, "avatar_background2Pink");
        colorKeysMap.put(key_avatar_backgroundInProfileBlue, "avatar_backgroundInProfileBlue");
        colorKeysMap.put(key_avatar_backgroundActionBarBlue, "avatar_backgroundActionBarBlue");
        colorKeysMap.put(key_avatar_actionBarSelectorBlue, "avatar_actionBarSelectorBlue");
        colorKeysMap.put(key_avatar_actionBarIconBlue, "avatar_actionBarIconBlue");
        colorKeysMap.put(key_avatar_subtitleInProfileBlue, "avatar_subtitleInProfileBlue");
        colorKeysMap.put(key_avatar_nameInMessageRed, "avatar_nameInMessageRed");
        colorKeysMap.put(key_avatar_nameInMessageOrange, "avatar_nameInMessageOrange");
        colorKeysMap.put(key_avatar_nameInMessageViolet, "avatar_nameInMessageViolet");
        colorKeysMap.put(key_avatar_nameInMessageGreen, "avatar_nameInMessageGreen");
        colorKeysMap.put(key_avatar_nameInMessageCyan, "avatar_nameInMessageCyan");
        colorKeysMap.put(key_avatar_nameInMessageBlue, "avatar_nameInMessageBlue");
        colorKeysMap.put(key_avatar_nameInMessagePink, "avatar_nameInMessagePink");
        colorKeysMap.put(key_actionBarDefault, "actionBarDefault");
        colorKeysMap.put(key_actionBarDefaultSelector, "actionBarDefaultSelector");
        colorKeysMap.put(key_actionBarWhiteSelector, "actionBarWhiteSelector");
        colorKeysMap.put(key_actionBarDefaultIcon, "actionBarDefaultIcon");
        colorKeysMap.put(key_actionBarActionModeDefault, "actionBarActionModeDefault");
        colorKeysMap.put(key_actionBarActionModeDefaultTop, "actionBarActionModeDefaultTop");
        colorKeysMap.put(key_actionBarActionModeDefaultIcon, "actionBarActionModeDefaultIcon");
        colorKeysMap.put(key_actionBarActionModeDefaultSelector, "actionBarActionModeDefaultSelector");
        colorKeysMap.put(key_actionBarDefaultTitle, "actionBarDefaultTitle");
        colorKeysMap.put(key_actionBarDefaultSubtitle, "actionBarDefaultSubtitle");
        colorKeysMap.put(key_actionBarDefaultSearch, "actionBarDefaultSearch");
        colorKeysMap.put(key_actionBarDefaultSearchPlaceholder, "actionBarDefaultSearchPlaceholder");
        colorKeysMap.put(key_actionBarDefaultSubmenuItem, "actionBarDefaultSubmenuItem");
        colorKeysMap.put(key_actionBarDefaultSubmenuItemIcon, "actionBarDefaultSubmenuItemIcon");
        colorKeysMap.put(key_actionBarDefaultSubmenuBackground, "actionBarDefaultSubmenuBackground");
        colorKeysMap.put(key_actionBarDefaultSubmenuSeparator, "actionBarDefaultSubmenuSeparator");
        colorKeysMap.put(key_actionBarTabActiveText, "actionBarTabActiveText");
        colorKeysMap.put(key_actionBarTabUnactiveText, "actionBarTabUnactiveText");
        colorKeysMap.put(key_actionBarTabLine, "actionBarTabLine");
        colorKeysMap.put(key_actionBarTabSelector, "actionBarTabSelector");
        colorKeysMap.put(key_actionBarDefaultArchived, "actionBarDefaultArchived");
        colorKeysMap.put(key_actionBarDefaultArchivedSelector, "actionBarDefaultArchivedSelector");
        colorKeysMap.put(key_actionBarDefaultArchivedIcon, "actionBarDefaultArchivedIcon");
        colorKeysMap.put(key_actionBarDefaultArchivedTitle, "actionBarDefaultArchivedTitle");
        colorKeysMap.put(key_actionBarDefaultArchivedSearch, "actionBarDefaultArchivedSearch");
        colorKeysMap.put(key_actionBarDefaultArchivedSearchPlaceholder, "actionBarDefaultSearchArchivedPlaceholder");
        colorKeysMap.put(key_actionBarBrowser, "actionBarBrowser");
        colorKeysMap.put(key_chats_onlineCircle, "chats_onlineCircle");
        colorKeysMap.put(key_chats_unreadCounter, "chats_unreadCounter");
        colorKeysMap.put(key_chats_unreadCounterMuted, "chats_unreadCounterMuted");
        colorKeysMap.put(key_chats_unreadCounterText, "chats_unreadCounterText");
        colorKeysMap.put(key_chats_name, "chats_name");
        colorKeysMap.put(key_chats_nameArchived, "chats_nameArchived");
        colorKeysMap.put(key_chats_secretName, "chats_secretName");
        colorKeysMap.put(key_chats_secretIcon, "chats_secretIcon");
        colorKeysMap.put(key_chats_pinnedIcon, "chats_pinnedIcon");
        colorKeysMap.put(key_chats_archiveBackground, "chats_archiveBackground");
        colorKeysMap.put(key_chats_archivePinBackground, "chats_archivePinBackground");
        colorKeysMap.put(key_chats_archiveIcon, "chats_archiveIcon");
        colorKeysMap.put(key_chats_archiveText, "chats_archiveText");
        colorKeysMap.put(key_chats_message, "chats_message");
        colorKeysMap.put(key_chats_messageArchived, "chats_messageArchived");
        colorKeysMap.put(key_chats_message_threeLines, "chats_message_threeLines");
        colorKeysMap.put(key_chats_draft, "chats_draft");
        colorKeysMap.put(key_chats_nameMessage, "chats_nameMessage");
        colorKeysMap.put(key_chats_nameMessageArchived, "chats_nameMessageArchived");
        colorKeysMap.put(key_chats_nameMessage_threeLines, "chats_nameMessage_threeLines");
        colorKeysMap.put(key_chats_nameMessageArchived_threeLines, "chats_nameMessageArchived_threeLines");
        colorKeysMap.put(key_chats_attachMessage, "chats_attachMessage");
        colorKeysMap.put(key_chats_actionMessage, "chats_actionMessage");
        colorKeysMap.put(key_chats_date, "chats_date");
        colorKeysMap.put(key_chats_pinnedOverlay, "chats_pinnedOverlay");
        colorKeysMap.put(key_chats_tabletSelectedOverlay, "chats_tabletSelectedOverlay");
        colorKeysMap.put(key_chats_sentCheck, "chats_sentCheck");
        colorKeysMap.put(key_chats_sentReadCheck, "chats_sentReadCheck");
        colorKeysMap.put(key_chats_sentClock, "chats_sentClock");
        colorKeysMap.put(key_chats_sentError, "chats_sentError");
        colorKeysMap.put(key_chats_sentErrorIcon, "chats_sentErrorIcon");
        colorKeysMap.put(key_chats_verifiedBackground, "chats_verifiedBackground");
        colorKeysMap.put(key_chats_verifiedCheck, "chats_verifiedCheck");
        colorKeysMap.put(key_chats_muteIcon, "chats_muteIcon");
        colorKeysMap.put(key_chats_mentionIcon, "chats_mentionIcon");
        colorKeysMap.put(key_chats_menuTopShadow, "chats_menuTopShadow");
        colorKeysMap.put(key_chats_menuTopShadowCats, "chats_menuTopShadowCats");
        colorKeysMap.put(key_chats_menuBackground, "chats_menuBackground");
        colorKeysMap.put(key_chats_menuItemText, "chats_menuItemText");
        colorKeysMap.put(key_chats_menuItemCheck, "chats_menuItemCheck");
        colorKeysMap.put(key_chats_menuItemIcon, "chats_menuItemIcon");
        colorKeysMap.put(key_chats_menuName, "chats_menuName");
        colorKeysMap.put(key_chats_menuPhone, "chats_menuPhone");
        colorKeysMap.put(key_chats_menuPhoneCats, "chats_menuPhoneCats");
        colorKeysMap.put(key_chats_menuTopBackgroundCats, "chats_menuTopBackgroundCats");
        colorKeysMap.put(key_chats_menuTopBackground, "chats_menuTopBackground");
        colorKeysMap.put(key_chats_actionIcon, "chats_actionIcon");
        colorKeysMap.put(key_chats_actionBackground, "chats_actionBackground");
        colorKeysMap.put(key_chats_actionPressedBackground, "chats_actionPressedBackground");
        colorKeysMap.put(key_chats_archivePullDownBackground, "chats_archivePullDownBackground");
        colorKeysMap.put(key_chats_archivePullDownBackgroundActive, "chats_archivePullDownBackgroundActive");
        colorKeysMap.put(key_chats_tabUnreadActiveBackground, "chats_tabUnreadActiveBackground");
        colorKeysMap.put(key_chats_tabUnreadUnactiveBackground, "chats_tabUnreadUnactiveBackground");
        colorKeysMap.put(key_chat_attachCheckBoxCheck, "chat_attachCheckBoxCheck");
        colorKeysMap.put(key_chat_attachCheckBoxBackground, "chat_attachCheckBoxBackground");
        colorKeysMap.put(key_chat_attachPhotoBackground, "chat_attachPhotoBackground");
        colorKeysMap.put(key_chat_attachActiveTab, "chat_attachActiveTab");
        colorKeysMap.put(key_chat_attachUnactiveTab, "chat_attachUnactiveTab");
        colorKeysMap.put(key_chat_attachPermissionImage, "chat_attachPermissionImage");
        colorKeysMap.put(key_chat_attachPermissionMark, "chat_attachPermissionMark");
        colorKeysMap.put(key_chat_attachPermissionText, "chat_attachPermissionText");
        colorKeysMap.put(key_chat_attachEmptyImage, "chat_attachEmptyImage");
        colorKeysMap.put(key_chat_inPollCorrectAnswer, "chat_inPollCorrectAnswer");
        colorKeysMap.put(key_chat_outPollCorrectAnswer, "chat_outPollCorrectAnswer");
        colorKeysMap.put(key_chat_inPollWrongAnswer, "chat_inPollWrongAnswer");
        colorKeysMap.put(key_chat_outPollWrongAnswer, "chat_outPollWrongAnswer");
        colorKeysMap.put(key_chat_attachIcon, "chat_attachIcon");
        colorKeysMap.put(key_chat_attachGalleryBackground, "chat_attachGalleryBackground");
        colorKeysMap.put(key_chat_attachGalleryText, "chat_attachGalleryText");
        colorKeysMap.put(key_chat_attachAudioBackground, "chat_attachAudioBackground");
        colorKeysMap.put(key_chat_attachAudioText, "chat_attachAudioText");
        colorKeysMap.put(key_chat_attachFileBackground, "chat_attachFileBackground");
        colorKeysMap.put(key_chat_attachFileText, "chat_attachFileText");
        colorKeysMap.put(key_chat_attachContactBackground, "chat_attachContactBackground");
        colorKeysMap.put(key_chat_attachContactText, "chat_attachContactText");
        colorKeysMap.put(key_chat_attachLocationBackground, "chat_attachLocationBackground");
        colorKeysMap.put(key_chat_attachLocationText, "chat_attachLocationText");
        colorKeysMap.put(key_chat_attachPollBackground, "chat_attachPollBackground");
        colorKeysMap.put(key_chat_attachPollText, "chat_attachPollText");
        colorKeysMap.put(key_chat_status, "chat_status");
        colorKeysMap.put(key_chat_inGreenCall, "chat_inDownCall");
        colorKeysMap.put(key_chat_outGreenCall, "chat_outUpCall");
        colorKeysMap.put(key_chat_inBubble, "chat_inBubble");
        colorKeysMap.put(key_chat_inBubbleSelected, "chat_inBubbleSelected");
        colorKeysMap.put(key_chat_inBubbleSelectedOverlay, "chat_inBubbleSelectedOverlay");
        colorKeysMap.put(key_chat_inBubbleShadow, "chat_inBubbleShadow");
        colorKeysMap.put(key_chat_outBubble, "chat_outBubble");
        colorKeysMap.put(key_chat_outBubbleGradient1, "chat_outBubbleGradient");
        colorKeysMap.put(key_chat_outBubbleGradient2, "chat_outBubbleGradient2");
        colorKeysMap.put(key_chat_outBubbleGradient3, "chat_outBubbleGradient3");
        colorKeysMap.put(key_chat_outBubbleGradientAnimated, "chat_outBubbleGradientAnimated");
        colorKeysMap.put(key_chat_outBubbleGradientSelectedOverlay, "chat_outBubbleGradientSelectedOverlay");
        colorKeysMap.put(key_chat_outBubbleSelected, "chat_outBubbleSelected");
        colorKeysMap.put(key_chat_outBubbleSelectedOverlay, "chat_outBubbleSelectedOverlay");
        colorKeysMap.put(key_chat_outBubbleShadow, "chat_outBubbleShadow");
        colorKeysMap.put(key_chat_messageTextIn, "chat_messageTextIn");
        colorKeysMap.put(key_chat_messageTextOut, "chat_messageTextOut");
        colorKeysMap.put(key_chat_messageLinkIn, "chat_messageLinkIn");
        colorKeysMap.put(key_chat_messageLinkOut, "chat_messageLinkOut");
        colorKeysMap.put(key_chat_serviceText, "chat_serviceText");
        colorKeysMap.put(key_chat_serviceLink, "chat_serviceLink");
        colorKeysMap.put(key_chat_serviceIcon, "chat_serviceIcon");
        colorKeysMap.put(key_chat_serviceBackground, "chat_serviceBackground");
        colorKeysMap.put(key_chat_serviceBackgroundSelected, "chat_serviceBackgroundSelected");
        colorKeysMap.put(key_chat_serviceBackgroundSelector, "chat_serviceBackgroundSelector");
        colorKeysMap.put(key_chat_muteIcon, "chat_muteIcon");
        colorKeysMap.put(key_chat_lockIcon, "chat_lockIcon");
        colorKeysMap.put(key_chat_outSentCheck, "chat_outSentCheck");
        colorKeysMap.put(key_chat_outSentCheckSelected, "chat_outSentCheckSelected");
        colorKeysMap.put(key_chat_outSentCheckRead, "chat_outSentCheckRead");
        colorKeysMap.put(key_chat_outSentCheckReadSelected, "chat_outSentCheckReadSelected");
        colorKeysMap.put(key_chat_outSentClock, "chat_outSentClock");
        colorKeysMap.put(key_chat_outSentClockSelected, "chat_outSentClockSelected");
        colorKeysMap.put(key_chat_inSentClock, "chat_inSentClock");
        colorKeysMap.put(key_chat_inSentClockSelected, "chat_inSentClockSelected");
        colorKeysMap.put(key_chat_mediaSentCheck, "chat_mediaSentCheck");
        colorKeysMap.put(key_chat_mediaSentClock, "chat_mediaSentClock");
        colorKeysMap.put(key_chat_inMediaIcon, "chat_inMediaIcon");
        colorKeysMap.put(key_chat_outMediaIcon, "chat_outMediaIcon");
        colorKeysMap.put(key_chat_inMediaIconSelected, "chat_inMediaIconSelected");
        colorKeysMap.put(key_chat_outMediaIconSelected, "chat_outMediaIconSelected");
        colorKeysMap.put(key_chat_mediaTimeBackground, "chat_mediaTimeBackground");
        colorKeysMap.put(key_chat_outViews, "chat_outViews");
        colorKeysMap.put(key_chat_outViewsSelected, "chat_outViewsSelected");
        colorKeysMap.put(key_chat_inViews, "chat_inViews");
        colorKeysMap.put(key_chat_inViewsSelected, "chat_inViewsSelected");
        colorKeysMap.put(key_chat_mediaViews, "chat_mediaViews");
        colorKeysMap.put(key_chat_outMenu, "chat_outMenu");
        colorKeysMap.put(key_chat_outMenuSelected, "chat_outMenuSelected");
        colorKeysMap.put(key_chat_inMenu, "chat_inMenu");
        colorKeysMap.put(key_chat_inMenuSelected, "chat_inMenuSelected");
        colorKeysMap.put(key_chat_mediaMenu, "chat_mediaMenu");
        colorKeysMap.put(key_chat_outInstant, "chat_outInstant");
        colorKeysMap.put(key_chat_outInstantSelected, "chat_outInstantSelected");
        colorKeysMap.put(key_chat_inInstant, "chat_inInstant");
        colorKeysMap.put(key_chat_inInstantSelected, "chat_inInstantSelected");
        colorKeysMap.put(key_chat_sentError, "chat_sentError");
        colorKeysMap.put(key_chat_sentErrorIcon, "chat_sentErrorIcon");
        colorKeysMap.put(key_chat_selectedBackground, "chat_selectedBackground");
        colorKeysMap.put(key_chat_previewDurationText, "chat_previewDurationText");
        colorKeysMap.put(key_chat_previewGameText, "chat_previewGameText");
        colorKeysMap.put(key_chat_inPreviewInstantText, "chat_inPreviewInstantText");
        colorKeysMap.put(key_chat_outPreviewInstantText, "chat_outPreviewInstantText");
        colorKeysMap.put(key_chat_secretTimeText, "chat_secretTimeText");
        colorKeysMap.put(key_chat_stickerNameText, "chat_stickerNameText");
        colorKeysMap.put(key_chat_botButtonText, "chat_botButtonText");
        colorKeysMap.put(key_chat_inForwardedNameText, "chat_inForwardedNameText");
        colorKeysMap.put(key_chat_outForwardedNameText, "chat_outForwardedNameText");
        colorKeysMap.put(key_chat_inPsaNameText, "chat_inPsaNameText");
        colorKeysMap.put(key_chat_outPsaNameText, "chat_outPsaNameText");
        colorKeysMap.put(key_chat_inViaBotNameText, "chat_inViaBotNameText");
        colorKeysMap.put(key_chat_outViaBotNameText, "chat_outViaBotNameText");
        colorKeysMap.put(key_chat_stickerViaBotNameText, "chat_stickerViaBotNameText");
        colorKeysMap.put(key_chat_inReplyLine, "chat_inReplyLine");
        colorKeysMap.put(key_chat_outReplyLine, "chat_outReplyLine");
        colorKeysMap.put(key_chat_stickerReplyLine, "chat_stickerReplyLine");
        colorKeysMap.put(key_chat_inReplyNameText, "chat_inReplyNameText");
        colorKeysMap.put(key_chat_outReplyNameText, "chat_outReplyNameText");
        colorKeysMap.put(key_chat_stickerReplyNameText, "chat_stickerReplyNameText");
        colorKeysMap.put(key_chat_inReplyMessageText, "chat_inReplyMessageText");
        colorKeysMap.put(key_chat_outReplyMessageText, "chat_outReplyMessageText");
        colorKeysMap.put(key_chat_inReplyMediaMessageText, "chat_inReplyMediaMessageText");
        colorKeysMap.put(key_chat_outReplyMediaMessageText, "chat_outReplyMediaMessageText");
        colorKeysMap.put(key_chat_inReplyMediaMessageSelectedText, "chat_inReplyMediaMessageSelectedText");
        colorKeysMap.put(key_chat_outReplyMediaMessageSelectedText, "chat_outReplyMediaMessageSelectedText");
        colorKeysMap.put(key_chat_stickerReplyMessageText, "chat_stickerReplyMessageText");
        colorKeysMap.put(key_chat_inPreviewLine, "chat_inPreviewLine");
        colorKeysMap.put(key_chat_outPreviewLine, "chat_outPreviewLine");
        colorKeysMap.put(key_chat_inSiteNameText, "chat_inSiteNameText");
        colorKeysMap.put(key_chat_outSiteNameText, "chat_outSiteNameText");
        colorKeysMap.put(key_chat_inContactNameText, "chat_inContactNameText");
        colorKeysMap.put(key_chat_outContactNameText, "chat_outContactNameText");
        colorKeysMap.put(key_chat_inContactPhoneText, "chat_inContactPhoneText");
        colorKeysMap.put(key_chat_inContactPhoneSelectedText, "chat_inContactPhoneSelectedText");
        colorKeysMap.put(key_chat_outContactPhoneText, "chat_outContactPhoneText");
        colorKeysMap.put(key_chat_outContactPhoneSelectedText, "chat_outContactPhoneSelectedText");
        colorKeysMap.put(key_chat_mediaProgress, "chat_mediaProgress");
        colorKeysMap.put(key_chat_inAudioProgress, "chat_inAudioProgress");
        colorKeysMap.put(key_chat_outAudioProgress, "chat_outAudioProgress");
        colorKeysMap.put(key_chat_inAudioSelectedProgress, "chat_inAudioSelectedProgress");
        colorKeysMap.put(key_chat_outAudioSelectedProgress, "chat_outAudioSelectedProgress");
        colorKeysMap.put(key_chat_mediaTimeText, "chat_mediaTimeText");
        colorKeysMap.put(key_chat_inAdminText, "chat_adminText");
        colorKeysMap.put(key_chat_inAdminSelectedText, "chat_adminSelectedText");
        colorKeysMap.put(key_chat_outAdminText, "chat_outAdminText");
        colorKeysMap.put(key_chat_outAdminSelectedText, "chat_outAdminSelectedText");
        colorKeysMap.put(key_chat_inTimeText, "chat_inTimeText");
        colorKeysMap.put(key_chat_outTimeText, "chat_outTimeText");
        colorKeysMap.put(key_chat_inTimeSelectedText, "chat_inTimeSelectedText");
        colorKeysMap.put(key_chat_outTimeSelectedText, "chat_outTimeSelectedText");
        colorKeysMap.put(key_chat_inAudioPerformerText, "chat_inAudioPerfomerText");
        colorKeysMap.put(key_chat_inAudioPerformerSelectedText, "chat_inAudioPerfomerSelectedText");
        colorKeysMap.put(key_chat_outAudioPerformerText, "chat_outAudioPerfomerText");
        colorKeysMap.put(key_chat_outAudioPerformerSelectedText, "chat_outAudioPerfomerSelectedText");
        colorKeysMap.put(key_chat_inAudioTitleText, "chat_inAudioTitleText");
        colorKeysMap.put(key_chat_outAudioTitleText, "chat_outAudioTitleText");
        colorKeysMap.put(key_chat_inAudioDurationText, "chat_inAudioDurationText");
        colorKeysMap.put(key_chat_outAudioDurationText, "chat_outAudioDurationText");
        colorKeysMap.put(key_chat_inAudioDurationSelectedText, "chat_inAudioDurationSelectedText");
        colorKeysMap.put(key_chat_outAudioDurationSelectedText, "chat_outAudioDurationSelectedText");
        colorKeysMap.put(key_chat_inAudioSeekbar, "chat_inAudioSeekbar");
        colorKeysMap.put(key_chat_inAudioCacheSeekbar, "chat_inAudioCacheSeekbar");
        colorKeysMap.put(key_chat_outAudioSeekbar, "chat_outAudioSeekbar");
        colorKeysMap.put(key_chat_outAudioCacheSeekbar, "chat_outAudioCacheSeekbar");
        colorKeysMap.put(key_chat_inAudioSeekbarSelected, "chat_inAudioSeekbarSelected");
        colorKeysMap.put(key_chat_outAudioSeekbarSelected, "chat_outAudioSeekbarSelected");
        colorKeysMap.put(key_chat_inAudioSeekbarFill, "chat_inAudioSeekbarFill");
        colorKeysMap.put(key_chat_outAudioSeekbarFill, "chat_outAudioSeekbarFill");
        colorKeysMap.put(key_chat_inVoiceSeekbar, "chat_inVoiceSeekbar");
        colorKeysMap.put(key_chat_outVoiceSeekbar, "chat_outVoiceSeekbar");
        colorKeysMap.put(key_chat_inVoiceSeekbarSelected, "chat_inVoiceSeekbarSelected");
        colorKeysMap.put(key_chat_outVoiceSeekbarSelected, "chat_outVoiceSeekbarSelected");
        colorKeysMap.put(key_chat_inVoiceSeekbarFill, "chat_inVoiceSeekbarFill");
        colorKeysMap.put(key_chat_outVoiceSeekbarFill, "chat_outVoiceSeekbarFill");
        colorKeysMap.put(key_chat_inFileProgress, "chat_inFileProgress");
        colorKeysMap.put(key_chat_outFileProgress, "chat_outFileProgress");
        colorKeysMap.put(key_chat_inFileProgressSelected, "chat_inFileProgressSelected");
        colorKeysMap.put(key_chat_outFileProgressSelected, "chat_outFileProgressSelected");
        colorKeysMap.put(key_chat_inFileNameText, "chat_inFileNameText");
        colorKeysMap.put(key_chat_outFileNameText, "chat_outFileNameText");
        colorKeysMap.put(key_chat_inFileInfoText, "chat_inFileInfoText");
        colorKeysMap.put(key_chat_outFileInfoText, "chat_outFileInfoText");
        colorKeysMap.put(key_chat_inFileInfoSelectedText, "chat_inFileInfoSelectedText");
        colorKeysMap.put(key_chat_outFileInfoSelectedText, "chat_outFileInfoSelectedText");
        colorKeysMap.put(key_chat_inFileBackground, "chat_inFileBackground");
        colorKeysMap.put(key_chat_outFileBackground, "chat_outFileBackground");
        colorKeysMap.put(key_chat_inFileBackgroundSelected, "chat_inFileBackgroundSelected");
        colorKeysMap.put(key_chat_outFileBackgroundSelected, "chat_outFileBackgroundSelected");
        colorKeysMap.put(key_chat_inVenueInfoText, "chat_inVenueInfoText");
        colorKeysMap.put(key_chat_outVenueInfoText, "chat_outVenueInfoText");
        colorKeysMap.put(key_chat_inVenueInfoSelectedText, "chat_inVenueInfoSelectedText");
        colorKeysMap.put(key_chat_outVenueInfoSelectedText, "chat_outVenueInfoSelectedText");
        colorKeysMap.put(key_chat_mediaInfoText, "chat_mediaInfoText");
        colorKeysMap.put(key_chat_linkSelectBackground, "chat_linkSelectBackground");
        colorKeysMap.put(key_chat_outLinkSelectBackground, "chat_outLinkSelectBackground");
        colorKeysMap.put(key_chat_textSelectBackground, "chat_textSelectBackground");
        colorKeysMap.put(key_chat_wallpaper, "chat_wallpaper");
        colorKeysMap.put(key_chat_wallpaper_gradient_to1, "chat_wallpaper_gradient_to");
        colorKeysMap.put(key_chat_wallpaper_gradient_to2, "key_chat_wallpaper_gradient_to2");
        colorKeysMap.put(key_chat_wallpaper_gradient_to3, "key_chat_wallpaper_gradient_to3");
        colorKeysMap.put(key_chat_wallpaper_gradient_rotation, "chat_wallpaper_gradient_rotation");
        colorKeysMap.put(key_chat_messagePanelBackground, "chat_messagePanelBackground");
        colorKeysMap.put(key_chat_messagePanelShadow, "chat_messagePanelShadow");
        colorKeysMap.put(key_chat_messagePanelText, "chat_messagePanelText");
        colorKeysMap.put(key_chat_messagePanelHint, "chat_messagePanelHint");
        colorKeysMap.put(key_chat_messagePanelCursor, "chat_messagePanelCursor");
        colorKeysMap.put(key_chat_messagePanelIcons, "chat_messagePanelIcons");
        colorKeysMap.put(key_chat_messagePanelSend, "chat_messagePanelSend");
        colorKeysMap.put(key_chat_messagePanelVoiceLock, "key_chat_messagePanelVoiceLock");
        colorKeysMap.put(key_chat_messagePanelVoiceLockBackground, "key_chat_messagePanelVoiceLockBackground");
        colorKeysMap.put(key_chat_messagePanelVoiceLockShadow, "key_chat_messagePanelVoiceLockShadow");
        colorKeysMap.put(key_chat_topPanelBackground, "chat_topPanelBackground");
        colorKeysMap.put(key_chat_topPanelClose, "chat_topPanelClose");
        colorKeysMap.put(key_chat_topPanelLine, "chat_topPanelLine");
        colorKeysMap.put(key_chat_topPanelTitle, "chat_topPanelTitle");
        colorKeysMap.put(key_chat_topPanelMessage, "chat_topPanelMessage");
        colorKeysMap.put(key_chat_addContact, "chat_addContact");
        colorKeysMap.put(key_chat_inLoader, "chat_inLoader");
        colorKeysMap.put(key_chat_inLoaderSelected, "chat_inLoaderSelected");
        colorKeysMap.put(key_chat_outLoader, "chat_outLoader");
        colorKeysMap.put(key_chat_outLoaderSelected, "chat_outLoaderSelected");
        colorKeysMap.put(key_chat_inLoaderPhoto, "chat_inLoaderPhoto");
        colorKeysMap.put(key_chat_mediaLoaderPhoto, "chat_mediaLoaderPhoto");
        colorKeysMap.put(key_chat_mediaLoaderPhotoSelected, "chat_mediaLoaderPhotoSelected");
        colorKeysMap.put(key_chat_mediaLoaderPhotoIcon, "chat_mediaLoaderPhotoIcon");
        colorKeysMap.put(key_chat_mediaLoaderPhotoIconSelected, "chat_mediaLoaderPhotoIconSelected");
        colorKeysMap.put(key_chat_inLocationBackground, "chat_inLocationBackground");
        colorKeysMap.put(key_chat_inLocationIcon, "chat_inLocationIcon");
        colorKeysMap.put(key_chat_outLocationIcon, "chat_outLocationIcon");
        colorKeysMap.put(key_chat_inContactBackground, "chat_inContactBackground");
        colorKeysMap.put(key_chat_inContactIcon, "chat_inContactIcon");
        colorKeysMap.put(key_chat_outContactBackground, "chat_outContactBackground");
        colorKeysMap.put(key_chat_outContactIcon, "chat_outContactIcon");
        colorKeysMap.put(key_chat_replyPanelIcons, "chat_replyPanelIcons");
        colorKeysMap.put(key_chat_replyPanelClose, "chat_replyPanelClose");
        colorKeysMap.put(key_chat_replyPanelName, "chat_replyPanelName");
        colorKeysMap.put(key_chat_replyPanelLine, "chat_replyPanelLine");
        colorKeysMap.put(key_chat_searchPanelIcons, "chat_searchPanelIcons");
        colorKeysMap.put(key_chat_searchPanelText, "chat_searchPanelText");
        colorKeysMap.put(key_chat_secretChatStatusText, "chat_secretChatStatusText");
        colorKeysMap.put(key_chat_fieldOverlayText, "chat_fieldOverlayText");
        colorKeysMap.put(key_chat_stickersHintPanel, "chat_stickersHintPanel");
        colorKeysMap.put(key_chat_botSwitchToInlineText, "chat_botSwitchToInlineText");
        colorKeysMap.put(key_chat_unreadMessagesStartArrowIcon, "chat_unreadMessagesStartArrowIcon");
        colorKeysMap.put(key_chat_unreadMessagesStartText, "chat_unreadMessagesStartText");
        colorKeysMap.put(key_chat_unreadMessagesStartBackground, "chat_unreadMessagesStartBackground");
        colorKeysMap.put(key_chat_inlineResultIcon, "chat_inlineResultIcon");
        colorKeysMap.put(key_chat_emojiPanelBackground, "chat_emojiPanelBackground");
        colorKeysMap.put(key_chat_emojiSearchBackground, "chat_emojiSearchBackground");
        colorKeysMap.put(key_chat_emojiSearchIcon, "chat_emojiSearchIcon");
        colorKeysMap.put(key_chat_emojiPanelShadowLine, "chat_emojiPanelShadowLine");
        colorKeysMap.put(key_chat_emojiPanelEmptyText, "chat_emojiPanelEmptyText");
        colorKeysMap.put(key_chat_emojiPanelIcon, "chat_emojiPanelIcon");
        colorKeysMap.put(key_chat_emojiBottomPanelIcon, "chat_emojiBottomPanelIcon");
        colorKeysMap.put(key_chat_emojiPanelIconSelected, "chat_emojiPanelIconSelected");
        colorKeysMap.put(key_chat_emojiPanelStickerPackSelector, "chat_emojiPanelStickerPackSelector");
        colorKeysMap.put(key_chat_emojiPanelStickerPackSelectorLine, "chat_emojiPanelStickerPackSelectorLine");
        colorKeysMap.put(key_chat_emojiPanelBackspace, "chat_emojiPanelBackspace");
        colorKeysMap.put(key_chat_emojiPanelTrendingTitle, "chat_emojiPanelTrendingTitle");
        colorKeysMap.put(key_chat_emojiPanelStickerSetName, "chat_emojiPanelStickerSetName");
        colorKeysMap.put(key_chat_emojiPanelStickerSetNameHighlight, "chat_emojiPanelStickerSetNameHighlight");
        colorKeysMap.put(key_chat_emojiPanelStickerSetNameIcon, "chat_emojiPanelStickerSetNameIcon");
        colorKeysMap.put(key_chat_emojiPanelTrendingDescription, "chat_emojiPanelTrendingDescription");
        colorKeysMap.put(key_chat_botKeyboardButtonText, "chat_botKeyboardButtonText");
        colorKeysMap.put(key_chat_botKeyboardButtonBackground, "chat_botKeyboardButtonBackground");
        colorKeysMap.put(key_chat_botKeyboardButtonBackgroundPressed, "chat_botKeyboardButtonBackgroundPressed");
        colorKeysMap.put(key_chat_emojiPanelNewTrending, "chat_emojiPanelNewTrending");
        colorKeysMap.put(key_chat_messagePanelVoicePressed, "chat_messagePanelVoicePressed");
        colorKeysMap.put(key_chat_messagePanelVoiceBackground, "chat_messagePanelVoiceBackground");
        colorKeysMap.put(key_chat_messagePanelVoiceDelete, "chat_messagePanelVoiceDelete");
        colorKeysMap.put(key_chat_messagePanelVoiceDuration, "chat_messagePanelVoiceDuration");
        colorKeysMap.put(key_chat_recordedVoicePlayPause, "chat_recordedVoicePlayPause");
        colorKeysMap.put(key_chat_recordedVoiceProgress, "chat_recordedVoiceProgress");
        colorKeysMap.put(key_chat_recordedVoiceProgressInner, "chat_recordedVoiceProgressInner");
        colorKeysMap.put(key_chat_recordedVoiceDot, "chat_recordedVoiceDot");
        colorKeysMap.put(key_chat_recordedVoiceBackground, "chat_recordedVoiceBackground");
        colorKeysMap.put(key_chat_recordVoiceCancel, "chat_recordVoiceCancel");
        colorKeysMap.put(key_chat_recordTime, "chat_recordTime");
        colorKeysMap.put(key_chat_messagePanelCancelInlineBot, "chat_messagePanelCancelInlineBot");
        colorKeysMap.put(key_chat_gifSaveHintText, "chat_gifSaveHintText");
        colorKeysMap.put(key_chat_gifSaveHintBackground, "chat_gifSaveHintBackground");
        colorKeysMap.put(key_chat_goDownButton, "chat_goDownButton");
        colorKeysMap.put(key_chat_goDownButtonIcon, "chat_goDownButtonIcon");
        colorKeysMap.put(key_chat_goDownButtonCounter, "chat_goDownButtonCounter");
        colorKeysMap.put(key_chat_goDownButtonCounterBackground, "chat_goDownButtonCounterBackground");
        colorKeysMap.put(key_chat_outTextSelectionHighlight, "chat_outTextSelectionHighlight");
        colorKeysMap.put(key_chat_inTextSelectionHighlight, "chat_inTextSelectionHighlight");
        colorKeysMap.put(key_chat_TextSelectionCursor, "chat_TextSelectionCursor");
        colorKeysMap.put(key_chat_outTextSelectionCursor, "chat_outTextSelectionCursor");
        colorKeysMap.put(key_chat_inBubbleLocationPlaceholder, "chat_inBubbleLocationPlaceholder");
        colorKeysMap.put(key_chat_outBubbleLocationPlaceholder, "chat_outBubbleLocationPlaceholder");
        colorKeysMap.put(key_chat_BlurAlpha, "chat_BlurAlpha");

        colorKeysMap.put(key_voipgroup_listSelector, "voipgroup_listSelector");
        colorKeysMap.put(key_voipgroup_inviteMembersBackground, "voipgroup_inviteMembersBackground");
        colorKeysMap.put(key_voipgroup_actionBar, "voipgroup_actionBar");
        colorKeysMap.put(key_voipgroup_actionBarItems, "voipgroup_actionBarItems");
        colorKeysMap.put(key_voipgroup_actionBarItemsSelector, "voipgroup_actionBarItemsSelector");
        colorKeysMap.put(key_voipgroup_actionBarUnscrolled, "voipgroup_actionBarUnscrolled");
        colorKeysMap.put(key_voipgroup_listViewBackgroundUnscrolled, "voipgroup_listViewBackgroundUnscrolled");
        colorKeysMap.put(key_voipgroup_lastSeenTextUnscrolled, "voipgroup_lastSeenTextUnscrolled");
        colorKeysMap.put(key_voipgroup_mutedIconUnscrolled, "voipgroup_mutedIconUnscrolled");
        colorKeysMap.put(key_voipgroup_nameText, "voipgroup_nameText");
        colorKeysMap.put(key_voipgroup_lastSeenText, "voipgroup_lastSeenText");
        colorKeysMap.put(key_voipgroup_listeningText, "voipgroup_listeningText");
        colorKeysMap.put(key_voipgroup_speakingText, "voipgroup_speakingText");
        colorKeysMap.put(key_voipgroup_mutedIcon, "voipgroup_mutedIcon");
        colorKeysMap.put(key_voipgroup_mutedByAdminIcon, "voipgroup_mutedByAdminIcon");
        colorKeysMap.put(key_voipgroup_listViewBackground, "voipgroup_listViewBackground");
        colorKeysMap.put(key_voipgroup_dialogBackground, "voipgroup_dialogBackground");
        colorKeysMap.put(key_voipgroup_leaveCallMenu, "voipgroup_leaveCallMenu");
        colorKeysMap.put(key_voipgroup_checkMenu, "voipgroup_checkMenu");
        colorKeysMap.put(key_voipgroup_soundButton, "voipgroup_soundButton");
        colorKeysMap.put(key_voipgroup_soundButtonActive, "voipgroup_soundButtonActive");
        colorKeysMap.put(key_voipgroup_soundButtonActiveScrolled, "voipgroup_soundButtonActiveScrolled");
        colorKeysMap.put(key_voipgroup_soundButton2, "voipgroup_soundButton2");
        colorKeysMap.put(key_voipgroup_soundButtonActive2, "voipgroup_soundButtonActive2");
        colorKeysMap.put(key_voipgroup_soundButtonActive2Scrolled, "voipgroup_soundButtonActive2Scrolled");
        colorKeysMap.put(key_voipgroup_leaveButton, "voipgroup_leaveButton");
        colorKeysMap.put(key_voipgroup_leaveButtonScrolled, "voipgroup_leaveButtonScrolled");
        colorKeysMap.put(key_voipgroup_muteButton, "voipgroup_muteButton");
        colorKeysMap.put(key_voipgroup_muteButton2, "voipgroup_muteButton2");
        colorKeysMap.put(key_voipgroup_muteButton3, "voipgroup_muteButton3");
        colorKeysMap.put(key_voipgroup_unmuteButton, "voipgroup_unmuteButton");
        colorKeysMap.put(key_voipgroup_unmuteButton2, "voipgroup_unmuteButton2");
        colorKeysMap.put(key_voipgroup_disabledButton, "voipgroup_disabledButton");
        colorKeysMap.put(key_voipgroup_disabledButtonActive, "voipgroup_disabledButtonActive");
        colorKeysMap.put(key_voipgroup_disabledButtonActiveScrolled, "voipgroup_disabledButtonActiveScrolled");
        colorKeysMap.put(key_voipgroup_connectingProgress, "voipgroup_connectingProgress");
        colorKeysMap.put(key_voipgroup_scrollUp, "voipgroup_scrollUp");
        colorKeysMap.put(key_voipgroup_searchPlaceholder, "voipgroup_searchPlaceholder");
        colorKeysMap.put(key_voipgroup_searchBackground, "voipgroup_searchBackground");
        colorKeysMap.put(key_voipgroup_searchText, "voipgroup_searchText");
        colorKeysMap.put(key_voipgroup_overlayGreen1, "voipgroup_overlayGreen1");
        colorKeysMap.put(key_voipgroup_overlayGreen2, "voipgroup_overlayGreen2");
        colorKeysMap.put(key_voipgroup_overlayBlue1, "voipgroup_overlayBlue1");
        colorKeysMap.put(key_voipgroup_overlayBlue2, "voipgroup_overlayBlue2");
        colorKeysMap.put(key_voipgroup_topPanelGreen1, "voipgroup_topPanelGreen1");
        colorKeysMap.put(key_voipgroup_topPanelGreen2, "voipgroup_topPanelGreen2");
        colorKeysMap.put(key_voipgroup_topPanelBlue1, "voipgroup_topPanelBlue1");
        colorKeysMap.put(key_voipgroup_topPanelBlue2, "voipgroup_topPanelBlue2");
        colorKeysMap.put(key_voipgroup_topPanelGray, "voipgroup_topPanelGray");
        colorKeysMap.put(key_voipgroup_overlayAlertGradientMuted, "voipgroup_overlayAlertGradientMuted");
        colorKeysMap.put(key_voipgroup_overlayAlertGradientMuted2, "voipgroup_overlayAlertGradientMuted2");
        colorKeysMap.put(key_voipgroup_overlayAlertGradientUnmuted, "voipgroup_overlayAlertGradientUnmuted");
        colorKeysMap.put(key_voipgroup_overlayAlertGradientUnmuted2, "voipgroup_overlayAlertGradientUnmuted2");
        colorKeysMap.put(key_voipgroup_overlayAlertMutedByAdmin, "voipgroup_overlayAlertMutedByAdmin");
        colorKeysMap.put(key_voipgroup_overlayAlertMutedByAdmin2, "kvoipgroup_overlayAlertMutedByAdmin2");
        colorKeysMap.put(key_voipgroup_mutedByAdminGradient, "voipgroup_mutedByAdminGradient");
        colorKeysMap.put(key_voipgroup_mutedByAdminGradient2, "voipgroup_mutedByAdminGradient2");
        colorKeysMap.put(key_voipgroup_mutedByAdminGradient3, "voipgroup_mutedByAdminGradient3");
        colorKeysMap.put(key_voipgroup_mutedByAdminMuteButton, "voipgroup_mutedByAdminMuteButton");
        colorKeysMap.put(key_voipgroup_mutedByAdminMuteButtonDisabled, "voipgroup_mutedByAdminMuteButtonDisabled");
        colorKeysMap.put(key_voipgroup_windowBackgroundWhiteInputField, "voipgroup_windowBackgroundWhiteInputField");
        colorKeysMap.put(key_voipgroup_windowBackgroundWhiteInputFieldActivated, "voipgroup_windowBackgroundWhiteInputFieldActivated");
        colorKeysMap.put(key_passport_authorizeBackground, "passport_authorizeBackground");
        colorKeysMap.put(key_passport_authorizeBackgroundSelected, "passport_authorizeBackgroundSelected");
        colorKeysMap.put(key_passport_authorizeText, "passport_authorizeText");
        colorKeysMap.put(key_profile_creatorIcon, "profile_creatorIcon");
        colorKeysMap.put(key_profile_title, "profile_title");
        colorKeysMap.put(key_profile_actionIcon, "profile_actionIcon");
        colorKeysMap.put(key_profile_actionBackground, "profile_actionBackground");
        colorKeysMap.put(key_profile_actionPressedBackground, "profile_actionPressedBackground");
        colorKeysMap.put(key_profile_verifiedBackground, "profile_verifiedBackground");
        colorKeysMap.put(key_profile_verifiedCheck, "profile_verifiedCheck");
        colorKeysMap.put(key_profile_status, "profile_status");
        colorKeysMap.put(key_profile_tabText, "profile_tabText");
        colorKeysMap.put(key_profile_tabSelectedText, "profile_tabSelectedText");
        colorKeysMap.put(key_profile_tabSelectedLine, "profile_tabSelectedLine");
        colorKeysMap.put(key_profile_tabSelector, "profile_tabSelector");
        colorKeysMap.put(key_sharedMedia_startStopLoadIcon, "sharedMedia_startStopLoadIcon");
        colorKeysMap.put(key_sharedMedia_linkPlaceholder, "sharedMedia_linkPlaceholder");
        colorKeysMap.put(key_sharedMedia_linkPlaceholderText, "sharedMedia_linkPlaceholderText");
        colorKeysMap.put(key_sharedMedia_photoPlaceholder, "sharedMedia_photoPlaceholder");
        colorKeysMap.put(key_featuredStickers_addedIcon, "featuredStickers_addedIcon");
        colorKeysMap.put(key_featuredStickers_buttonProgress, "featuredStickers_buttonProgress");
        colorKeysMap.put(key_featuredStickers_addButton, "featuredStickers_addButton");
        colorKeysMap.put(key_featuredStickers_addButtonPressed, "featuredStickers_addButtonPressed");
        colorKeysMap.put(key_featuredStickers_removeButtonText, "featuredStickers_removeButtonText");
        colorKeysMap.put(key_featuredStickers_buttonText, "featuredStickers_buttonText");
        colorKeysMap.put(key_featuredStickers_unread, "featuredStickers_unread");
        colorKeysMap.put(key_stickers_menu, "stickers_menu");
        colorKeysMap.put(key_stickers_menuSelector, "stickers_menuSelector");
        colorKeysMap.put(key_changephoneinfo_image2, "changephoneinfo_image2");
        colorKeysMap.put(key_groupcreate_hintText, "groupcreate_hintText");
        colorKeysMap.put(key_groupcreate_cursor, "groupcreate_cursor");
        colorKeysMap.put(key_groupcreate_sectionShadow, "groupcreate_sectionShadow");
        colorKeysMap.put(key_groupcreate_sectionText, "groupcreate_sectionText");
        colorKeysMap.put(key_groupcreate_spanText, "groupcreate_spanText");
        colorKeysMap.put(key_groupcreate_spanBackground, "groupcreate_spanBackground");
        colorKeysMap.put(key_groupcreate_spanDelete, "groupcreate_spanDelete");
        colorKeysMap.put(key_contacts_inviteBackground, "contacts_inviteBackground");
        colorKeysMap.put(key_contacts_inviteText, "contacts_inviteText");
        colorKeysMap.put(key_login_progressInner, "login_progressInner");
        colorKeysMap.put(key_login_progressOuter, "login_progressOuter");
        colorKeysMap.put(key_picker_enabledButton, "picker_enabledButton");
        colorKeysMap.put(key_picker_disabledButton, "picker_disabledButton");
        colorKeysMap.put(key_picker_badge, "picker_badge");
        colorKeysMap.put(key_picker_badgeText, "picker_badgeText");
        colorKeysMap.put(key_location_sendLocationBackground, "location_sendLocationBackground");
        colorKeysMap.put(key_location_sendLocationIcon, "location_sendLocationIcon");
        colorKeysMap.put(key_location_sendLocationText, "location_sendLocationText");
        colorKeysMap.put(key_location_sendLiveLocationBackground, "location_sendLiveLocationBackground");
        colorKeysMap.put(key_location_sendLiveLocationIcon, "location_sendLiveLocationIcon");
        colorKeysMap.put(key_location_sendLiveLocationText, "location_sendLiveLocationText");
        colorKeysMap.put(key_location_liveLocationProgress, "location_liveLocationProgress");
        colorKeysMap.put(key_location_placeLocationBackground, "location_placeLocationBackground");
        colorKeysMap.put(key_location_actionIcon, "location_actionIcon");
        colorKeysMap.put(key_location_actionActiveIcon, "location_actionActiveIcon");
        colorKeysMap.put(key_location_actionBackground, "location_actionBackground");
        colorKeysMap.put(key_location_actionPressedBackground, "location_actionPressedBackground");
        colorKeysMap.put(key_dialog_liveLocationProgress, "dialog_liveLocationProgress");
        colorKeysMap.put(key_files_folderIcon, "files_folderIcon");
        colorKeysMap.put(key_files_folderIconBackground, "files_folderIconBackground");
        colorKeysMap.put(key_files_iconText, "files_iconText");
        colorKeysMap.put(key_sessions_devicesImage, "sessions_devicesImage");
        colorKeysMap.put(key_calls_callReceivedGreenIcon, "calls_callReceivedGreenIcon");
        colorKeysMap.put(key_calls_callReceivedRedIcon, "calls_callReceivedRedIcon");
        colorKeysMap.put(key_undo_background, "undo_background");
        colorKeysMap.put(key_undo_cancelColor, "undo_cancelColor");
        colorKeysMap.put(key_undo_infoColor, "undo_infoColor");
        colorKeysMap.put(key_sheet_scrollUp, "key_sheet_scrollUp");
        colorKeysMap.put(key_sheet_other, "key_sheet_other");
        colorKeysMap.put(key_player_actionBarSelector, "player_actionBarSelector");
        colorKeysMap.put(key_player_actionBarTitle, "player_actionBarTitle");
        colorKeysMap.put(key_player_actionBarSubtitle, "player_actionBarSubtitle");
        colorKeysMap.put(key_player_actionBarItems, "player_actionBarItems");
        colorKeysMap.put(key_player_background, "player_background");
        colorKeysMap.put(key_player_time, "player_time");
        colorKeysMap.put(key_player_progressBackground, "player_progressBackground");
        colorKeysMap.put(key_player_progressCachedBackground, "key_player_progressCachedBackground");
        colorKeysMap.put(key_player_progress, "player_progress");
        colorKeysMap.put(key_player_button, "player_button");
        colorKeysMap.put(key_player_buttonActive, "player_buttonActive");

        colorKeysMap.put(key_statisticChartSignature, "statisticChartSignature");
        colorKeysMap.put(key_statisticChartSignatureAlpha, "statisticChartSignatureAlpha");
        colorKeysMap.put(key_statisticChartHintLine, "statisticChartHintLine");
        colorKeysMap.put(key_statisticChartActiveLine, "statisticChartActiveLine");
        colorKeysMap.put(key_statisticChartInactivePickerChart, "statisticChartInactivePickerChart");
        colorKeysMap.put(key_statisticChartActivePickerChart, "statisticChartActivePickerChart");
        colorKeysMap.put(key_statisticChartRipple, "statisticChartRipple");
        colorKeysMap.put(key_statisticChartBackZoomColor, "statisticChartBackZoomColor");
        colorKeysMap.put(key_statisticChartChevronColor, "statisticChartChevronColor");
        colorKeysMap.put(key_statisticChartLine_blue, "statisticChartLine_blue");
        colorKeysMap.put(key_statisticChartLine_green, "statisticChartLine_green");
        colorKeysMap.put(key_statisticChartLine_red, "statisticChartLine_red");
        colorKeysMap.put(key_statisticChartLine_golden, "statisticChartLine_golden");
        colorKeysMap.put(key_statisticChartLine_lightblue, "statisticChartLine_lightblue");
        colorKeysMap.put(key_statisticChartLine_lightgreen, "statisticChartLine_lightgreen");
        colorKeysMap.put(key_statisticChartLine_orange, "statisticChartLine_orange");
        colorKeysMap.put(key_statisticChartLine_indigo, "statisticChartLine_indigo");
        colorKeysMap.put(key_statisticChartLine_purple, "statisticChartLine_purple");
        colorKeysMap.put(key_statisticChartLine_cyan, "statisticChartLine_cyan");
        colorKeysMap.put(key_statisticChartLineEmpty, "statisticChartLineEmpty");
        colorKeysMap.put(key_color_lightblue, "color_lightblue");
        colorKeysMap.put(key_color_blue, "color_blue");
        colorKeysMap.put(key_color_green, "color_green");
        colorKeysMap.put(key_color_lightgreen, "color_lightgreen");
        colorKeysMap.put(key_color_red, "color_red");
        colorKeysMap.put(key_color_orange, "color_orange");
        colorKeysMap.put(key_color_yellow, "color_yellow");
        colorKeysMap.put(key_color_purple, "color_purple");
        colorKeysMap.put(key_color_cyan, "color_cyan");
        colorKeysMap.put(key_chat_outReactionButtonBackground, "chat_outReactionButtonBackground");
        colorKeysMap.put(key_chat_inReactionButtonBackground, "chat_inReactionButtonBackground");
        colorKeysMap.put(key_chat_outReactionButtonText, "chat_outReactionButtonText");
        colorKeysMap.put(key_chat_inReactionButtonText, "chat_inReactionButtonText");
        colorKeysMap.put(key_chat_inReactionButtonTextSelected, "chat_inReactionButtonTextSelected");
        colorKeysMap.put(key_chat_outReactionButtonTextSelected, "chat_outReactionButtonTextSelected");
        colorKeysMap.put(key_premiumGradient0, "premiumGradient0");
        colorKeysMap.put(key_premiumGradient1, "premiumGradient1");
        colorKeysMap.put(key_premiumGradient2, "premiumGradient2");
        colorKeysMap.put(key_premiumGradient3, "premiumGradient3");
        colorKeysMap.put(key_premiumGradient4, "premiumGradient4");
        colorKeysMap.put(key_premiumGradientBackground1, "premiumGradientBackground1");
        colorKeysMap.put(key_premiumGradientBackground2, "premiumGradientBackground2");
        colorKeysMap.put(key_premiumGradientBackground3, "premiumGradientBackground3");
        colorKeysMap.put(key_premiumGradientBackground4, "premiumGradientBackground4");
        colorKeysMap.put(key_premiumGradientBackgroundOverlay, "premiumGradientBackgroundOverlay");
        colorKeysMap.put(key_premiumStartSmallStarsColor, "premiumStartSmallStarsColor");
        colorKeysMap.put(key_premiumStartGradient1, "premiumStarGradient1");
        colorKeysMap.put(key_premiumStartGradient2, "premiumStarGradient2");
        colorKeysMap.put(key_premiumStartSmallStarsColor2, "premiumStartSmallStarsColor2");
        colorKeysMap.put(key_premiumGradientBottomSheet1, "premiumGradientBottomSheet1");
        colorKeysMap.put(key_premiumGradientBottomSheet2, "premiumGradientBottomSheet2");
        colorKeysMap.put(key_premiumGradientBottomSheet3, "premiumGradientBottomSheet3");
        colorKeysMap.put(key_topics_unreadCounter, "topics_unreadCounter");
        colorKeysMap.put(key_topics_unreadCounterMuted, "topics_unreadCounterMuted");
        return colorKeysMap;
    }

    private static HashMap<String, Integer> createColorKeysStringMap() {
        if (colorKeysMap == null) {
            colorKeysMap = createColorKeysMap();
        }
        HashMap<String, Integer> map = new HashMap<>();
        for (int i = 0; i < colorKeysMap.size(); i++) {
            map.put(colorKeysMap.valueAt(i), colorKeysMap.keyAt(i));
        }
        return map;
    }

    public static int stringKeyToInt(String key) {
        if (colorKeysStringMap == null) {
            colorKeysStringMap = createColorKeysStringMap();
        }
        Integer i = colorKeysStringMap.get(key);
        if (i == null) {
            return -1;
        } else {
            return colorKeysStringMap.get(key);
        }
    }


    public static String getStringName(int currentKey) {
        if (colorKeysMap == null) {
            colorKeysMap = createColorKeysMap();
        }
        return colorKeysMap.get(currentKey);
    }
}
