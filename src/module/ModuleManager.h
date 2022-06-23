#pragma once

#include "Module.h"
#include "events/EventChannel.h"
#include "util/Log.h"

/*
 * Varargs - Tuple concept from Hat (Babbaj iirc)
 */
template<typename... T_MODS>
class ModuleManager {
	std::tuple<T_MODS...> modules;
	std::array<Module*, sizeof...(T_MODS)> pointers = {&std::get<T_MODS>(modules)...};

public:
	ModuleManager() {
		static_assert((... && std::is_base_of<Module, T_MODS>::value));

		// Command listener
		EventChannel::registerStaticListener<ChatEvent>([&](auto &ce) {
			if (ce.text.front() == '.') {
				ce.cancel();
				auto command = ce.text; // Copy so history keeps original
				command.erase(command.begin()); // Remove prefix
				handle(ce, command); // Handle
			}
		}, EventPriority::High);

	};

	Module *get(std::string alias) {
		// Strip input identically as done during construction of modules
		std::transform(alias.begin(), alias.end(), alias.begin(), ::tolower);
		alias.erase(std::remove_if(alias.begin(), alias.end(), ::isspace), alias.end());

		Module *best = nullptr;
		unsigned char bestLen = 255;
		for (auto sub : pointers) {
			if (sub->strippedName.find(alias, 0) == 0) {
				unsigned char len = sub->strippedName.length() - alias.length();
				if (len <= bestLen) {
					best = sub;
					bestLen = len;
				}
			}
		}
		return best;
	};

	template<typename T>
	T& get() {
		return std::get<T>(modules);
	}

	template<typename T>
	void forEachMod(T fn) {
		(fn(std::get<T_MODS>(modules)), ...);
	}

	auto begin() {
		return pointers.begin();
	}

	auto end() {
		return pointers.end();
	}

	auto cbegin() const {
		return pointers.cbegin();
	}

	auto cend() const {
		return pointers.cend();
	}

	// Command handler
	void handle(chat_t chatEvent, std::string command) {
		// TODO: Don't split commas inside quotes oof
		static const std::regex cmdsRx{";+"}, argsRx{R"('[^']*'|"[^"]*"|[^"' ]+)"};

		// Tokenize commands
		std::sregex_token_iterator it{command.begin(), command.end(), cmdsRx, -1}, end;
		for (; it != end; ++it) {
			// Tokenize arguments
			auto s = it->str();
			std::regex_iterator<std::string::const_iterator> it{s.begin(), s.end(), argsRx}, end;

			// Check if arguments are empty
			if (it == end)
				continue;

			// Put matches in vector and remove trailing quotes
			std::vector<std::string> args;
			std::transform(it, end, std::back_inserter(args), [](auto m) {
				auto s = m.str();
				if (s.front() == '"' || s.front() == '\'') {
					s.erase(s.begin());
					s.pop_back();
				}
				return s;
			});

			if (Module *mod = get(args.front())) {
				args.erase(args.begin());
				mod->execute(chatEvent, args);
				continue;
			}

			Log() << "Unknown command \"" << args.front() << "\".";
		}
	}
};

