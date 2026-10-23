#pragma once

#include <map>
#include <string>

#include "LocalizationGuiText.h"

namespace BrowserPresentation
{
    inline std::string text(std::string const& canonical)
    {
        static std::map<std::string, std::string> const keys = {
            {"Simulations", "browser.simulations"},
            {"Genomes", "browser.genomes"},
            {"Featured", "browser.featured"},
            {"Community", "browser.community"},
            {"Private", "browser.private"},
            {"Refresh", "browser.refresh"},
            {"Change name or description", "browser.changenameordescription"},
            {"Change folder name", "browser.changefoldername"},
            {"Change visibility", "browser.changevisibility"},
            {"Expand all folders", "browser.expandallfolders"},
            {"Collapse all folders", "browser.collapseallfolders"},
            {"Gallery view", "browser.galleryview"},
            {"Table view", "browser.tableview"},
            {"Open ALIEN Discord server", "browser.openaliendiscordserver"},
            {"Show the account menu", "browser.showtheaccountmenu"},
            {"Log in", "browser.login"},
            {"Log out", "browser.logout"},
            {"More", "browser.more"},
            {"Choose a reaction", "browser.chooseareaction"},
            {"Sort by", "browser.sortby"},
            {"Most reactions", "browser.mostreactions"},
            {"Newest", "browser.newest"},
            {"Most downloads", "browser.mostdownloads"},
            {"Card size", "browser.cardsize"},
            {"First page", "browser.firstpage"},
            {"Previous page", "browser.previouspage"},
            {"Next page", "browser.nextpage"},
            {"Last page", "browser.lastpage"},
            {"NEW", "browser.new"},
            {"Visible in Featured", "browser.visibleinfeatured"},
            {"Visible in Community", "browser.visibleincommunity"},
            {"User", "browser.user"},
            {"User name", "browser.username"},
            {"Timestamp", "browser.timestamp"},
            {"Reactions", "browser.reactions"},
            {"Downloads", "browser.downloads"},
            {"World size", "browser.worldsize"},
            {"Objects", "browser.objects"},
            {"Cells", "browser.cells"},
            {"File size", "browser.filesize"},
            {"Version", "browser.version"},
            {"loading...", "browser.loading"},
            {"no preview", "browser.nopreview"},
            {"Loading...", "browser.loading"},
            {"Add a reaction", "browser.addareaction"},
            {"Download", "browser.download"},
            {"Name", "browser.name"},
            {"Description", "browser.description"},
            {"Width", "browser.width"},
            {"Height", "browser.height"},
            {"Simulators", "browser.simulators"},
            {"GPU model", "browser.gpumodel"},
            {"not shared", "browser.notshared"},
            {"Share yours to see", "browser.shareyourstosee"},
            {"Log in to see", "browser.logintosee"},
            {"Time spent", "browser.timespent"},
            {"Reactions received", "browser.reactionsreceived"},
            {"Reactions given", "browser.reactionsgiven"},
            {"Your private workspace", "browser.yourprivateworkspace"},
            {"Log in to see the simulations and genomes you uploaded.", "browser.logintoseethesimulationsandgenomesyouuploaded"},
            {"Login or register", "browser.loginorregister"},
            {"Log in or register", "browser.loginorregister"},
            {"Join the ALIEN community", "browser.jointhealiencommunity"},
            {"Upload your own simulations, react to those of others and get a private workspace. It's free.", "browser.invitation.body"},
            {"Hide the invitation until the next program start", "browser.hidetheinvitationuntilthenextprogramstart"},
            {"Folder name", "browser.foldername"},
            {"Not logged in", "browser.notloggedin"},
            {"In order to share and upvote simulations you need to log in.", "browser.inordertoshareandupvotesimulationsyouneedtologin"},
            {"Filter (case insensitive)", "browser.filtercaseinsensitive"},
            {" KB", "browser.kb"},
            {" K", "browser.k"},
            {" Bytes", "browser.bytes"},
            {"h", "browser.h"},
            {"m", "browser.m"},
            {"Change visibility: Community → Private and Private → Community", "browser.changevisibilitycommunityprivateandprivatecommunity"},
            {"Do you really want to delete the selected item?", "browser.doyoureallywanttodeletetheselecteditem"},
            {"Do you really want to delete the selected folder?", "browser.doyoureallywanttodeletetheselectedfolder"},
            {"Delete", "browser.delete"},
            {"Downloading ...", "browser.downloading"},
            {"Uploading ...", "browser.uploading"},
            {"Replacing ...", "browser.replacing"},
            {"Deleting ...", "browser.deleting"},
            {"Applying changes ...", "browser.applyingchanges"},
            {"Changing visibility ...", "browser.changingvisibility"},
            {"OK", "common.ok"},
            {"Cancel", "common.cancel"},
            {"Yes", "common.yes"},
            {"No", "common.no"},
            {"Error", "common.error"},
            {"Desc (optional)", "genome.upload.description"},
            {"Upload simulation", "browser.uploadsimulation"},
            {"Replace simulation", "browser.replacesimulation"},
            {"Delete selected simulation", "browser.deleteselectedsimulation"},
            {"Show the simulations as tiles with preview pictures.", "browser.showthesimulationsastileswithpreviewpictures"},
            {"Show the simulations as a sortable table with folders.", "browser.showthesimulationsasasortabletablewithfolders"},
            {"Replace the selected simulation with the one that is currently open. The name, description and reactions will be preserved.",
             "browser.replace.simulation.help"},
            {"Do you really want to replace the following simulation with the currently opened simulation?", "browser.replace.simulation.confirm"},
            {"Upload your current simulation to the server and make it visible in the browser. You can choose whether you want to share it with the community "
             "or whether it should only be visible in your own workspace.\nIf you have already selected a folder, your simulation will be uploaded there. If "
             "you have selected a simulation, its folder and name will be suggested.",
             "browser."
             "uploadyourcurrentsimulationtotheserverandmakeitvisibleinthebrowseryoucanchoosewhetheryouwanttoshareitwiththecommunityorwhetheritshouldonlybevisib"
             "leinyourownworkspaceifyouhavealreadyselectedafolderyoursimulationwillbeuploadedthereifyouhaveselectedasimulationitsfolderandnamewillbesuggested"},
            {"Upload genome", "browser.uploadgenome"},
            {"Replace genome", "browser.replacegenome"},
            {"Delete selected genome", "browser.deleteselectedgenome"},
            {"Show the genomes as tiles with preview pictures.", "browser.showthegenomesastileswithpreviewpictures"},
            {"Show the genomes as a sortable table with folders.", "browser.showthegenomesasasortabletablewithfolders"},
            {"Replace the selected genome with the one that is currently open. The name, description and reactions will be preserved.",
             "browser.replace.genome.help"},
            {"Do you really want to replace the following genome with the currently opened genome?", "browser.replace.genome.confirm"},
            {"Upload your current genome to the server and make it visible in the browser. You can choose whether you want to share it with the community or "
             "whether it should only be visible in your own workspace.\nIf you have already selected a folder, your genome will be uploaded there. If you have "
             "selected a genome, its folder and name will be suggested.",
             "browser."
             "uploadyourcurrentgenometotheserverandmakeitvisibleinthebrowseryoucanchoosewhetheryouwanttoshareitwiththecommunityorwhetheritshouldonlybevisiblein"
             "yourownworkspaceifyouhavealreadyselectedafolderyourgenomewillbeuploadedthereifyouhaveselectedagenomeitsfolderandnamewillbesuggested"},
            {"Number of ALIEN instances that fetched the browser list in the last 25 minutes, including those without a login. Every running instance does "
             "this on startup and then every 20 minutes.\n\nInstances are told apart by a hash of their IP address that the server keeps only in memory for 25 "
             "minutes and never stores. Instances sharing one IP address count once. Instances where 'Count me as active' is disabled in the network settings "
             "are not included.",
             "browser.help.activesimulators"},
            {"This workspace is curated by the alien-project and contains the simulations that come along with the released versions. They cover a wide range "
             "and exploit different features.",
             "browser.help.featuredworkspace"},
            {"All logged-in users can share their simulations and genomes here. The files stored in this workspace are visible to all users.",
             "browser.help.communityworkspace"},
            {"Each user account has its own private space. The simulations and genomes are only visible to the logged-in user.",
             "browser.help.privateworkspace"},
            {"Log in or create a new account to upload your own simulations and genomes and to react to those of others.", "browser.help.loginchip"},
            {"The GPU models are only visible to logged-in users who share their own GPU model.", "browser.help.gpuhiddenloggedout"},
            {"The GPU models are only visible if you share your own GPU model.", "browser.help.gpuhiddennotshared"},
            {"Other users will be able to see in the browser window that you have the following graphics card: ", "browser.help.sharegpu"},
        };
        return tr(keys.at(canonical));
    }

    inline std::string column(std::string const& canonical)
    {
        return text(canonical) + "###" + canonical;
    }

    inline std::string count(std::string const& key, int count)
    {
        auto suffix = count % 10 == 1 && count % 100 != 11                                 ? "one"
            : count % 10 >= 2 && count % 10 <= 4 && (count % 100 < 12 || count % 100 > 14) ? "few"
                                                                                           : "many";
        return LocalizationGuiText::format(key + "." + suffix, {{"count", std::to_string(count)}});
    }
}
